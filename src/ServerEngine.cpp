#include "ServerEngine.hpp"
#include "Log.hpp"

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
/**
 * @details bind() by default only binds to loacl IP addresses.
*/
int ServerEngine::setup_listening_socket(const ServerBlock& sb)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int reuse = 1;
	if (sockfd == -1 || !make_non_blocking(sockfd) || setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
	{
		std::ostringstream ss; ss << "Failed to create listening socket. Errno: " << errno;
		Log::log(ss.str(), ERROR);
		return (1);
	}
	sockaddr_in socket_addr;
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(sb.get_host());
	// socket_addr.sin_addr.s_addr = INADDR_ANY;
	socket_addr.sin_port = htons(sb.get_port());  // ensures endianness of network default, big endian

	if (bind(sockfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1)
	{
		// optional: remove serverblock that failed to bind from server_blocks to spare search time later?
		std::ostringstream ss; ss << "Failed to bind to host " << Config::ft_inet_ntoa(sb.get_host()) << ":" << sb.get_port()
			<< ". Errno: " << errno << ". Error: " << strerror(errno) << ".";
		Log::log(ss.str(), ERROR);
		close(sockfd);
		return (1);
	}
	if (listen(sockfd, 10) == -1)
	{
		std::cerr << "Failed to listen on socket. Errno: " << errno
			<< ". Errno: " << errno << ". " << strerror(errno) << "." << std::endl;
		close(sockfd);
		return (1);
	}

	pfd_info info = {};
	info.type = LISTENER_SOCKET;
	info.sockaddr = socket_addr;
	info.host = sb.get_host();
	info.port = sb.get_port();
	info.max_client_body = sb.get_max_client_body();
	pfd_info_map[sockfd] = info;

	std::ostringstream ss; ss << "Set up listener_fd no. " << sockfd << " on " << Utils::host_to_str(sb.get_host()) << ":" << sb.get_port();
	Log::log(ss.str(), DEBUG);

	return (0);
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
		perror("Failed to grab connection. Accept error");
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

	// Full engine procedure of adding and mapping a new client : updated to not initiate a Request here due to lifecycle of client
	pfds.push_back(fd);
	pfds_vector_modified = true;
	pfd_info info = {};
	info.type = CLIENT_CONNECTION_SOCKET;
	info.host = meta.host;
	info.port = meta.port;
	info.max_client_body = meta.max_client_body;
	info.reqs_idx = UNINITIALIZED;
	info.last_active = time(NULL);
	info.had_at_least_one_req_processed = false;
	pfd_info_map[client] = info;

	std::stringstream ss;
	ss << "New client requesting to connect from " << Utils::host_to_str(ntohl(meta.sockaddr.sin_addr.s_addr))
		<< ":" << meta.sockaddr.sin_port << ", accepted on FD " << client;
	Log::log(ss.str(), INFO);
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
	std::cout << "Forgetting client on socket FD: " << pfds_it->fd << std::endl;
	if (close(pfds_it->fd) == -1)
		perror("close");
	pfds.erase(pfds_it);
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
	ssize_t	nbytes;

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

void ServerEngine::initialize_new_request_if_no_active_one(std::map<int, pfd_info>::iterator& meta_it)
{
	if (meta_it->second.reqs_idx == UNINITIALIZED)
	{
		reqs.push_back(Request(meta_it->second.host, meta_it->second.port));
		meta_it->second.reqs_idx = reqs.size() - 1;
	}
}

void ServerEngine::liberate_client_for_next_request(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
    int idx = meta_it->second.reqs_idx;
    if (idx != UNINITIALIZED && idx < static_cast<int>(reqs.size())) {
        reqs.erase(reqs.begin() + idx);
    }
    meta_it->second.reqs_idx = UNINITIALIZED;
    meta_it->second.had_at_least_one_req_processed = true;
    pfds_it->events = POLLIN;
}

void ServerEngine::update_client_activity_timestamp(std::map<int, pfd_info>::iterator& meta_it)
{
	meta_it->second.last_active = time(NULL);
}


/* Once it reads CRLF CRLF it begings parsing the headers and then reads body if applicable, then processes the request */
void ServerEngine::read_from_client_fd(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int		idx = meta_it->second.reqs_idx;
	char	buf[BUF_SZ + 1];
	ssize_t	nbytes;
	uint32_t total_read;

	memset(buf, 0, BUF_SZ + 1);

	nbytes = recv(pfds_it->fd, buf, BUF_SZ, MSG_DONTWAIT);
	total_read = nbytes + reqs[idx].get_request_str().size();

	if (nbytes <= 0 || total_read > meta_it->second.max_client_body)  // hangup or error or sb disallows read size
	{
		if (nbytes == 0)
		{
			std::cout << "poll: socket " << pfds_it->fd << " hung up, orderly shutdown\n";
			forget_client(pfds_it, meta_it);
		}
		else if (nbytes == -1)
		{
			std::perror("recv");
			initiate_error_response(pfds_it, idx, CODE_503);
		}
		else
			initiate_error_response(pfds_it, idx, CODE_413);
		return;
	}

	if (reqs[idx].done_reading_headers())
	{
		size_t body_size = reqs[idx].get_request_body_raw().size();
		size_t content_length = reqs[idx].get_content_length();
		if (body_size > content_length)
			initiate_error_response(pfds_it, idx, CODE_413);
		for (ssize_t i = 0; i < nbytes; i++)
			reqs[idx].append_byte_to_body(buf[i]);
		// std::cout << "DEBUG MANUAL done_reading_headers() block, appended:" << std::endl;
		// for (ssize_t i = 0; i < nbytes; i++)
		// 	std::cout << buf[i];
		// std::cout << std::endl;
		std::cout << "DEBUG MANUAL nbytes from reading_body: " << nbytes << std::endl;
	}
	else
	{
		std::string this_buffer(buf);
		// TODO: optimize with substring and .rfind()
		// if (reqs[idx].get_request_str().find("\r\n\r\n") != std::string::npos)
		size_t crlf_begin = this_buffer.find("\r\n\r\n");
		if (crlf_begin == std::string::npos)
		{
			reqs[idx].append_to_request_str(buf);
		}
		else
		{
			reqs[idx].switch_to_reading_body();
			// buf[crlf_begin + 4] = 0;
			std::string buf_as_str(buf + 0, buf + crlf_begin + 4);
			// std::cout << "DEBUG MANUAL entire buf:" << std::endl << buf;
			// std::cout << "DEBUG MANUAL buf_as_str:" << std::endl << buf_as_str;
			reqs[idx].append_to_request_str(buf_as_str);
			for (ssize_t i = crlf_begin + 4; i < nbytes; i++)
				reqs[idx].append_byte_to_body(buf[i]);

			reqs[idx].parse();
			// is there a redirection? -> redirect
			// is there a cgi? -> add it to pfds

			// std::string appended(buf + crlf_begin + 4, buf + nbytes);
			// std::cout << "DEBUG MANUAL appended to body:" << std::endl << appended << std::endl;
		}
		std::cout << "DEBUG MANUAL nbytes: " << nbytes << std::endl;
		// return;
	}
	update_client_activity_timestamp(meta_it);

	if (reqs[idx].done_reading_headers() && static_cast<size_t>(reqs[idx].get_request_body_raw().size()) == static_cast<size_t>(reqs[idx].get_content_length()))
	{
		// std::cout << "DEBUG MANUAL entered nbytes < BUF_SZ block. nbytes: " << nbytes << ".buf:" << std::endl << buf << std::endl;
		std::cout << "REQUEST:" << std::endl << reqs[idx].get_request_str() << std::endl;
		std::cout << "BODY:" << std::endl << reqs[idx].get_request_body_as_str() << std::endl;
		std::cout << "----BODY-END----" << std::endl; 
		// while (recv(pfds_it->fd, buf, BUF_SZ, MSG_DONTWAIT) > 0)  // NO god no
		// 	;
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
		// pfds_vector_modified = true;  // no, not really?
		return;
	}
}

void ServerEngine::write_to_client(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
    size_t sz_to_send = BUF_SZ;
    int idx = meta_it->second.reqs_idx;
    if (idx == UNINITIALIZED || idx >= static_cast<int>(reqs.size())) {
        forget_client(pfds_it, meta_it);
        return;
    }
    int response_size = reqs[idx].get_response().size();
    int sent_so_far = reqs[idx].get_total_sent();

    if (response_size - sent_so_far < BUF_SZ) {
        if (response_size - sent_so_far < 0)
            std::cerr << "Critical error identified: sz_to_send less than 0" << std::endl;
        sz_to_send = response_size - sent_so_far;
    }

	if (sz_to_send)
	{
		std::string str_to_send = reqs[idx].get_response().substr(reqs[idx].get_total_sent());
		if (send(pfds_it->fd, str_to_send.c_str(), sz_to_send, MSG_DONTWAIT) == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				std::cerr << "Socket buffer full or would block, retrying send()" << std::endl;
				return;
			}
			else if (errno == EPIPE)
			{
				std::cerr << "Client closed connection (EPIPE), closing socket: " << pfds_it->fd << std::endl;
				std::cout << reqs[idx].get_request_str() << std::endl;
				forget_client(pfds_it, meta_it);  // Close the socket properly
				return;
			}
			else
			{
				perror("send");  // LOG
				std::cerr << "sz_to_send: " << sz_to_send << ", total size: " << response_size  << ", sent_so_far: " << sent_so_far << std::endl;
				// TODO: depending on request attributes, handle removal differently?
			}
		}
		reqs[idx].increment_total_sent_by(sz_to_send);
		update_client_activity_timestamp(meta_it);
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
			liberate_client_for_next_request(pfds_it, meta_it);

		// pfds_vector_modified = true;  // will reset to pfds.begin()  --> ahh this was preventing other PFDs being reached!
	}
}

void ServerEngine::process_eof_on_pipe(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int idx = meta_it->second.reqs_idx;

	std::cout << "Processing EOF on pipe, close" << std::endl;
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
		if (meta_it->second.had_at_least_one_req_processed)  // silent close
		{
			std::cout << "Client connection " << pfds_it->fd << " has timed out, closing the connection silently." << std::endl;
			forget_client(pfds_it, meta_it);
			return ;
		}
		std::cout << "Client connection " << pfds_it->fd << " has timed out, responding 408." << std::endl;
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

	result = processor.handleMethod(req, server_blocks);
	req.set_response(result);
	pfds_it->events = POLLOUT;  // get ready for writing
}

bool ServerEngine::is_client_and_timed_out(const pfd_info& pfd_meta)
{
	if (pfd_meta.type == CLIENT_CONNECTION_SOCKET)
	{
		time_t now = time(NULL);
		long delta = now - pfd_meta.last_active;
		if (delta >= CONNECTION_TIMEOUT / 1000)
			return true;
	}
	return false;
}

void	ServerEngine::remove_failed_blocks(std::vector<ServerBlock> &server_blocks, std::vector<int> &failed_indexes)
{
	if (failed_indexes.empty())
		return ;
	std::vector<ServerBlock>::reverse_iterator it;
	int i = server_blocks.size() - 1;
	for (it = server_blocks.rbegin(); it != server_blocks.rend(); it++)
	{
		if (!failed_indexes.empty() && failed_indexes.back() == i)
		{
			server_blocks.erase((it.base() - 1));
			std::clog << "Erasing server block at index " << i << std::endl;
			failed_indexes.pop_back();
		}
		i--;
	}
	// Log::log(server_blocks);
}

void ServerEngine::run()
{
	std::signal(SIGINT, signal_handler); // handles Ctrl+C
	std::signal(SIGPIPE, SIG_IGN);  // ignore SIGPIPE so that we can handle errno == EPIPE ourselves
	std::vector<int> failed_indexes;

	for (size_t i = 0; i < server_blocks.size(); i++)
	{
		if (setup_listening_socket(server_blocks[i]))
			failed_indexes.push_back(i);
	}

	// remove server_blocks[i] that fail
	remove_failed_blocks(server_blocks, failed_indexes);
	if (server_blocks.empty())
		return (Log::log("No server blocks set up. Exiting...", ERROR));

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
			try
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
					{
						initialize_new_request_if_no_active_one(meta_it);
						read_from_client_fd(pfds_it, meta_it);  // has the try / catch
					}
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
			catch (std::exception& e)
			{
				Log::log(e.what(), ERROR);
				pfds_it++;
			}
		}
	}
	size_t i;
	for (i = 0; i < pfds.size(); i++)
	{
		if (close(pfds[i].fd) == -1)
			Log::log(strerror(errno), ERROR);
	}
	std::ostringstream ss; ss << "Closed " << i << " pfds before exiting";
	Log::log(ss.str(), DEBUG);
}
