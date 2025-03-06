#include "ServerEngine.hpp"

ServerEngine::ServerEngine(std::vector<ServerBlock>& initialized_server_blocks) :
		pfds_vector_modified(false)
{
	// for (int i = 0; i < 5; i++)
	// 	ports.push_back(9991 + i);
	this->server_blocks = initialized_server_blocks;
}

ServerEngine::~ServerEngine() {}

void ServerEngine::signal_handler(int signal)
{
	g_signal = signal;
}

bool ServerEngine::make_non_blocking(int &fd)
{
	int flags = O_NONBLOCK;

	int status = fcntl(fd, F_SETFL, flags);
	if (status == -1)
	{
		std::perror("fcntl F_SETFL O_NONBLOCK");
		return (false);
	}
	return (true);
}

/* project-wide htonl() and htons() should be contained to this function only */
void ServerEngine::setup_listening_socket(const ServerBlock& sb)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1 || !make_non_blocking(sockfd))
	{
		std::cerr << "Failed to create listening socket. Errno: " << errno << std::endl;
		return;
	}
	sockaddr_in socket_addr;
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(sb.get_host());
	// socket_addr.sin_addr.s_addr = INADDR_ANY;
	socket_addr.sin_port = htons(sb.get_port());  // ensures endianness of network default, big endian

	if (bind(sockfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1)
	{
		// optional: remove serverblock that failed to bind from server_blocks to spare search time later?
		std::cerr << "Failed to bind to port " << sb.get_port() << ". Errno:" << errno << std::endl;
		return;
	}
	if (listen(sockfd, 10) == -1)
	{
		std::cerr << "Failed to listen on socket. Errno: " << errno << std::endl;
		return;
	}

	pfd_info info = {};
	info.type = LISTENER_SOCKET;
	info.sockaddr = socket_addr;
	info.host = sb.get_host();
	info.port = sb.get_port();
	pfd_info_map[sockfd] = info;

	std::cout << "Set up listener_fd no. " << sockfd << " for port no. " << ntohs(socket_addr.sin_port) << std::endl;
}

void ServerEngine::init_listener_pfds()
{
	for (std::map<int, pfd_info>::iterator it = pfd_info_map.begin(); it != pfd_info_map.end(); it++)
	{
		struct pollfd fd;
		fd.fd = it->first;
		fd.events = POLLIN;
		if (pfds.size() >= MAX_SERVER_BLOCKS)
		{
			std::cerr << "Max connections reached." << std::endl;
			return;
		}

		pfds.push_back(fd);
	}
}

/** accept() returns a new client_fd. We initiate a new request and map the client to pfd_info_map with:
 * (1) type CLIENT_CONNECTION_SOCKET
 * (2) reqs_idx of the newly initiated Request in vector<Request>
 * (3) port and host not really needed on a CLIENT_CONNECTION_SOCKET ? The respective request will have them?
*/
void ServerEngine::accept_client(int listener_fd, pfd_info meta)
{
	int addrlen = sizeof(meta.sockaddr);
	int client = accept(listener_fd, (struct sockaddr*)&meta.sockaddr, (socklen_t*)&addrlen);
	if (client == -1 || !make_non_blocking(client))
	{
		std::cerr << "Failed to grab connection. Errn: " << errno << std::endl;
		return ;
	}
	struct pollfd fd;
	fd.fd = client;
	fd.events = POLLIN;
	if (pfds.size() >= MAX_CONNECTIONS)
	{
		std::cerr << "Max connections reached." << std::endl;
		return ;
	}

	// Full engine procedure of adding and mapping a new client
	pfds.push_back(fd);
	pfds_vector_modified = true;
	reqs.push_back(Request(meta.port, meta.host));
	pfd_info info = {};
	info.type = CLIENT_CONNECTION_SOCKET;
	info.reqs_idx = reqs.size() - 1;
	info.last_active = time(NULL);
	pfd_info_map[client] = info;

	std::cout << "New client accepted on FD " << client << std::endl;
}

/**
 * (1) close the client_fd
 * (2) remove the pfd from vector
 * (3) remove the req from vector
 * (4) remove the mapping of the fd and the reqs_idx
 * Finally, set the flag that pfd vector has been modified
 */
void ServerEngine::forget_client(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	if (close(pfds_it->fd) == -1)
		perror("close");
	pfds.erase(pfds_it);
	reqs.erase(reqs.begin() + meta_it->second.reqs_idx);
	pfd_info_map.erase(meta_it);
	pfds_vector_modified = true;
}

void ServerEngine::initiate_error_response(std::vector<pollfd>::iterator& pfds_it, int idx, int code)
{
	reqs[idx].set_response_status(code);
	reqs[idx].set_response(ErrorPageGenerator::createErrorPage(reqs[idx], server_blocks));
	pfds_it->events = POLLOUT;
}

void ServerEngine::print_pfds()
{
	std::vector<pollfd>::iterator it = pfds.begin();
	while (it != pfds.end())
	{
		std::cout << "fd: " << it->fd << ", type: " << pfd_info_map[it->fd].type << std::endl;
		it++;
	}
}


void ServerEngine::read_from_cgi_pipe(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	char	buf[BUF_SZ];
	int		nbytes;

	memset(buf, 0, BUF_SZ);
	nbytes = read(pfds_it->fd, buf, BUF_SZ);  // this should never turn out zero when POLLIN
	if (nbytes < 0)
	{
		std::perror("read(pipe_fd)");
		// TODO: message the client instead of forgetting them
		close(pfds_it->fd);
		pfds.erase(pfds_it);
		pfd_info_map.erase(meta_it);
		pfds_vector_modified = true;
		return;
	}
	reqs[meta_it->second.reqs_idx].append_to_cgi_output(buf);
}

/* Once it reads CRLF CRLF it begings parsing and processing the request */
void ServerEngine::read_from_client_fd(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int		idx = meta_it->second.reqs_idx;
	char	buf[BUF_SZ];
	int		nbytes;

	memset(buf, 0, BUF_SZ);

	nbytes = recv(pfds_it->fd, buf, BUF_SZ, MSG_DONTWAIT);
	if (nbytes <= 0)  // hangup or error
	{
		if (nbytes == 0)
		{
			std::cout << "poll: socket " << pfds_it->fd << " hung up, orderly shutdown\n";
			forget_client(pfds_it, meta_it);
		}
		else
		{
			std::perror("recv");
			initiate_error_response(pfds_it, idx, CODE_503);
		}
		return;
	}
	reqs[idx].append_to_request_str(buf);
	// TODO: optimize with substring and .rfind()
	if (reqs[idx].get_request_str().find("\r\n\r\n") != std::string::npos) // if proper HTTP ending
	{
		while (recv(pfds_it->fd, buf, BUF_SZ, MSG_DONTWAIT) > 0)  // NO god no
			;
		// TODO: parse headers, determine if we need to read the body as well
		try
		{
			process_request(pfds_it, reqs[idx]);
		}
		catch (std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;  // LOG
			if (dynamic_cast<RequestException*>(&e))
				initiate_error_response(pfds_it, idx, dynamic_cast<RequestException*>(&e)->code());
			else
				initiate_error_response(pfds_it, idx, CODE_500);
			return;
		}
		// OBSOLETE:
		// set_response(pfds_it, meta_it->second.reqs_idx);
		pfds_vector_modified = true;  // no, not really?
		return;
	}
}

void ServerEngine::write_to_client(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	size_t	sz_to_send = BUF_SZ;
	int		idx = meta_it->second.reqs_idx;
	size_t	response_size = reqs[idx].get_response().size();
	size_t	sent_so_far = reqs[idx].get_total_sent();

	if (response_size - sent_so_far < BUF_SZ)
		sz_to_send = response_size - sent_so_far;

	if (sz_to_send)
	{
		std::string str_to_send = reqs[idx].get_response().substr(reqs[idx].get_total_sent());
		if (send(pfds_it->fd, str_to_send.c_str(), sz_to_send, MSG_DONTWAIT) == -1)
		{
			perror("send");  // LOG
			std::cerr << "sz_to_send: " << sz_to_send << ", total size: " << response_size  << ", sent_so_far: " << sent_so_far << std::endl;
			// TODO: depending on request attributes, handle removal differently?
		}
		reqs[idx].increment_total_sent_by(sz_to_send);
	}
	else  // sz_to_send == 0
	{
		if (reqs[idx].timed_out())
		{
			std::cout << "timeout: client on socket " << pfds_it->fd << ", closing connection\n";
			close(pfds_it->fd);
			pfds.erase(pfds_it);
			if (!reqs[idx].should_await_reconnection())
			{
				reqs.erase(reqs.begin() + idx);
				pfd_info_map.erase(meta_it);
			}
		}
		else
		{
			reqs[idx].reset();  // TODO: obsolete, rather remove the request
			pfds_it->events = POLLIN;
		}
		pfds_vector_modified = true;  // will reset to pfds.begin()  --> ahh this was preventing other PFDs being reached!
	}
}

void ServerEngine::process_eof_on_pipe(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int idx = meta_it->second.reqs_idx;

	close(pfds_it->fd);
	pfds.erase(pfds_it);
	// do not erase pfd_info_map, it's staying

	reqs[idx].set_cgi_status(AWAIT_CLIENT_RECONNECT);
	pfds_vector_modified = true;
}

void ServerEngine::process_unorderly_hangup(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	std::cout << "poll: socket " << pfds_it->fd << " hung up, unorderly shutdown\n";
	forget_client(pfds_it, meta_it);
}

void ServerEngine::process_connection_timeout(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int idx = meta_it->second.reqs_idx;

	try
	{
		if (!reqs[idx].get_request_str().empty())
			initiate_error_response(pfds_it, idx, CODE_400);
		else  // request is empty, we can close client connection
			initiate_error_response(pfds_it, idx, CODE_408);  // this would need more time per NGINX
		reqs[idx].flag_the_timeout();
		pfds_vector_modified = true;  // needed here? Cause I suspect it might block actually
	}
	catch (RequestException& e) {
		std::cout << e.what() << std::endl;
	}
}

void ServerEngine::process_request(std::vector<pollfd>::iterator& pfds_it, Request& req)
{
	RequestProcessor processor;
	std::string result;

	req.parse();
	// is there a redirection? -> redirect
	// is there a cgi? -> add it to pfds
	result = processor.handleMethod(req, server_blocks);
	req.set_response(result);
	pfds_it->events = POLLOUT;
}

bool ServerEngine::is_client_and_timed_out(const pfd_info& pfd_meta)
{
	if (pfd_meta.type == CLIENT_CONNECTION_SOCKET)
	{
		time_t now = time(NULL);
		if (now - pfd_meta.last_active >= CONNECTION_TIMEOUT)
			return true;
	}
	return false;
}

void ServerEngine::run()
{
	std::signal(SIGINT, signal_handler); // handles Ctrl+C

	for (size_t i = 0; i < server_blocks.size(); i++)
		setup_listening_socket(server_blocks[i]);

	init_listener_pfds();

	while (!g_signal)
	{
		int events_count = poll(&pfds[0], pfds.size(), CONNECTION_TIMEOUT);
		if (events_count == -1)
		{
			//if SIGINT (Ctrl+C) is received, exit gracefully
			if (errno == EINTR) {
				std::cout << "\nSignal received. Exiting..." << std::endl;
				break;
			}
			std::cout << "Poll failed. Errn: " << errno << ". Trying again..." << std::endl;
			continue;
		}

		std::vector<pollfd>::iterator pfds_it = pfds.begin();
		while (!g_signal && pfds_it != pfds.end())
		{
			std::map<int, pfd_info>::iterator	meta_it = pfd_info_map.find(pfds_it->fd);
			struct pfd_info&					pfd_meta = meta_it->second;

			if (pfd_meta.type == LISTENER_SOCKET)
			{
				if (pfds_it->revents & POLLIN)
					accept_client(pfds_it->fd, pfd_meta);
			}
			else if (pfds_it->revents & POLLIN)
			{
				if (pfd_meta.type == CGI_PIPE)
					read_from_cgi_pipe(pfds_it, meta_it);
				else
					read_from_client_fd(pfds_it, meta_it);  // has the try / catch
			}
			else if (pfds_it->revents & POLLOUT)
			{
				write_to_client(pfds_it, meta_it);
			}
			else if (pfds_it->revents & POLLHUP)
			{
				if (pfd_meta.type == CGI_PIPE)
					process_eof_on_pipe(pfds_it, meta_it);
				else
					process_unorderly_hangup(pfds_it, meta_it);
			}
			else if (pfds_it->revents & (POLLERR | POLLNVAL))
			{
				std::cout << "POLLERR | POLLNVAL" << std::endl;
				forget_client(pfds_it, meta_it);
			}
			else if (is_client_and_timed_out(pfd_meta))  // no flag AND is client
				process_connection_timeout(pfds_it, meta_it);

			if (pfds_vector_modified)
			{
				pfds_vector_modified = false;
				break;
			}
			pfds_it++;
		}
	}
}
