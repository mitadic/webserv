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
		std::ostringstream oss; oss << "Failed to create listening socket. Errno: " << errno;
		Log::log(oss.str(), ERROR);
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
		std::ostringstream oss; oss << "Failed to bind to host " << Utils::ft_inet_ntoa(sb.get_host()) << ":" << sb.get_port()
			<< ". Errno: " << errno << ". Error: " << strerror(errno) << ".";
		Log::log(oss.str(), ERROR);
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

	std::ostringstream oss; oss << "Set up listener_fd no. " << sockfd << " on " << Utils::host_to_str(sb.get_host()) << ":" << sb.get_port();
	Log::log(oss.str(), DEBUG);

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
	info.has_had_response = false;
	pfd_info_map[client] = info;

	std::stringstream ss;
	ss << "New client requesting to connect from " << Utils::host_to_str(ntohl(meta.sockaddr.sin_addr.s_addr))
		<< ":" << meta.sockaddr.sin_port << ", accepted on FD " << client;
	Log::log(ss.str(), INFO);
}

/**
 * (1) close the fd
 * (2) remove the pfd from vector
 * (3) remove the mapping of the fd and the reqs_idx
 * Finally, set the flag that pfd vector has been modified
 */
void ServerEngine::terminate_pfd(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	if (close(pfds_it->fd) == -1)
		perror("close");
	pfds.erase(pfds_it);
	pfd_info_map.erase(meta_it);
	pfds_vector_modified = true;
}

void ServerEngine::forget_client(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	std::ostringstream oss; oss << "Forgetting client on socket FD: " << pfds_it->fd;
	Log::log(oss.str(), DEBUG);
	terminate_pfd(pfds_it, meta_it);
}

/* terminate_pfd associated with CGI_PIPE_IN and set cgi's pipe_in[1] to -1 */
void ServerEngine::discard_cgi_pipe_in(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int reqs_idx = meta_it->second.reqs_idx;

	std::ostringstream oss; oss << "Discarding cgi_pipe_in with FD: " << pfds_it->fd;
	Log::log(oss.str(), INFO);
	terminate_pfd(pfds_it, meta_it);
	reqs[reqs_idx].cgi->pipe_in[1] = UNINITIALIZED;
}

/* terminate_pfd associated with CGI_PIPE_OUT and set cgi's pipe_out[0] to -1 */
void ServerEngine::discard_cgi_pipe_out(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int reqs_idx = meta_it->second.reqs_idx;

	std::ostringstream oss; oss << "Discarding cgi_pipe_out with FD: " << pfds_it->fd;
	Log::log(oss.str(), INFO);
	terminate_pfd(pfds_it, meta_it);
	reqs[reqs_idx].cgi->pipe_out[0] = UNINITIALIZED;
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

/* locate the pipe_out's meta by req_idx, in order to identify the peer cgi_pipe_out's pfd by fd, in order to set pipe_out pfd to POLLIN */
void ServerEngine::set_matching_cgi_pipe_out_to_pollin(std::map<int, pfd_info>::iterator& meta_it)
{
	for (std::map<int, pfd_info>::iterator pipe_out_meta_it = pfd_info_map.begin(); pipe_out_meta_it != pfd_info_map.end(); pipe_out_meta_it++)
	{
		if (pipe_out_meta_it->second.type == CGI_PIPE_OUT && pipe_out_meta_it->second.reqs_idx == meta_it->second.reqs_idx)  // found the cgi_pipe_out
		{
			for (std::vector<pollfd>::iterator pipe_out_pfd_it = pfds.begin(); pipe_out_pfd_it != pfds.end(); pipe_out_pfd_it++)
			{
				if (pipe_out_meta_it->first == pipe_out_pfd_it->fd)
				{
					pipe_out_pfd_it->events = POLLIN;
					return;
				}
			}
		}
	}
	std::cerr << "Critical error identified: found no matching cgi_pipe_out after finishing writing to cgi_pipe_in" << std::endl;
}

void ServerEngine::write_to_cgi_pipe_in(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	size_t sz_to_send = BUF_SZ;
	int idx = meta_it->second.reqs_idx;
	int body_size = reqs[idx].get_content_length();
	int sent_so_far = reqs[idx].get_total_sent();

	if (body_size - sent_so_far < BUF_SZ)
	{
		if (body_size - sent_so_far < 0)
			std::cerr << "Critical error identified write()-ing to cgi_pipe_in: sz_to_send less than 0. sz_to_send: " << sz_to_send << ", body_size: " << body_size << std::endl;
		sz_to_send = body_size - sent_so_far;
	}

	if (sz_to_send)
	{
		std::string str_to_send = reqs[idx].get_request_body_as_str().substr(reqs[idx].get_total_sent());
		if (write(pfds_it->fd, str_to_send.c_str(), sz_to_send) == -1)
		{
			perror("write() to cgi_pipe_in");  // LOG
			std::cerr << "sz_to_send: " << sz_to_send << ", total size: " << body_size  << ", sent_so_far: " << sent_so_far << std::endl;
			// TODO: handle removal
		}
		std::cout << "DEBUG: successfully written to cgi_pipe_in: " << str_to_send << std::endl;
		reqs[idx].increment_total_sent_by(sz_to_send);
		update_client_activity_timestamp(meta_it);
	}
	else  // sz_to_send == 0
	{
		reqs[idx].set_total_sent(0);  // reset to reuse same attribute for when writing to client_fd
		// set_matching_cgi_pipe_out_to_pollin(meta_it);
		discard_cgi_pipe_in(pfds_it, meta_it);
	}
}

void ServerEngine::read_from_cgi_pipe_out(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	char	buf[BUF_SZ];
	ssize_t	nbytes;

	memset(buf, 0, BUF_SZ);
	nbytes = read(pfds_it->fd, buf, BUF_SZ);  // this should never turn out zero when POLLIN
	if (nbytes < 0)
	{
		std::perror("read(pipe_fd)");
		// TODO: message the client instead of forgetting them?
		discard_cgi_pipe_out(pfds_it, meta_it);
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

/* Does not rm the client's meta, just resets the reqs_idx to UNINITIALIZED */
void ServerEngine::liberate_client_for_next_request(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int idx = meta_it->second.reqs_idx;
	if (idx != UNINITIALIZED && idx < static_cast<int>(reqs.size())) {
		reqs.erase(reqs.begin() + idx);
	}
	meta_it->second.reqs_idx = UNINITIALIZED;
	meta_it->second.has_had_response = true;
	pfds_it->events = POLLIN;

	std::ostringstream oss; oss << "client_fd " << pfds_it->fd << " liberated for new requests";
	Log::log(oss.str(), DEBUG);
}

void ServerEngine::update_client_activity_timestamp(std::map<int, pfd_info>::iterator& meta_it)
{
	meta_it->second.last_active = time(NULL);
}


/* Process hangup (0) or error (-1) */
void ServerEngine::process_read_failure(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it, const int& idx, const ssize_t& nbytes)
{
	if (nbytes == 0)
	{
		std::ostringstream oss; oss << "poll: socket " << pfds_it->fd << " hung up, orderly shutdown";
		Log::log(oss.str(), DEBUG);
		forget_client(pfds_it, meta_it);
	}
	else if (nbytes == -1)
	{
		// This scenario includes "Connection reset by peer", and without checking errno...better to drop client and req instead of 503?
		std::perror("recv");
		initiate_error_response(pfds_it, idx, CODE_503);
	}
}

/* Read headers, add any buf spillover to body, and parse and validate the headers before proceeding */
int ServerEngine::read_headers(std::vector<pollfd>::iterator& pfds_it, const int& idx, const char* buf, const ssize_t& nbytes)
{
	ssize_t the_trail_from_prev = 0;
	if (reqs[idx].get_request_str().size() >= 3)
		the_trail_from_prev = 3;
	size_t tail_start = reqs[idx].get_request_str().size() - the_trail_from_prev;

	// this_buffer == 3 tailing chars of request_str (if applicable) + current buf, to catch broken CRLFCRLF
	std::string this_buffer = reqs[idx].get_request_str().substr(tail_start);
	this_buffer += buf;
	size_t crlf_begin = this_buffer.find("\r\n\r\n");
	if (crlf_begin == std::string::npos)
	{
		reqs[idx].append_to_request_str(buf);
	}
	else
	{
		reqs[idx].switch_to_reading_body();
		std::string buf_as_str(buf + 0, buf + crlf_begin + the_trail_from_prev);
		reqs[idx].append_to_request_str(buf_as_str);
		for (ssize_t i = crlf_begin + 4; i < nbytes + the_trail_from_prev; i++)	// 3: tailsize; 4: rnrn size
			reqs[idx].append_byte_to_body(buf[i - the_trail_from_prev]);

		try {
			reqs[idx].parse();
			// is there a cgi? -> add it to pfds
		}
		catch (RequestException& e) {
			Log::log("Corrupt headers, initiating early closing to prevent processing unknown socket buffer", WARNING);
			Log::log(static_cast<std::ostringstream&>(std::ostringstream() << "Corruption reason: " << e.what()).str(), DEBUG);
			reqs[idx].flag_that_we_should_close_early();
			initiate_error_response(pfds_it, idx, e.code());
			return 1;
		}
	}
	return 0;
}

/* Read body */
int ServerEngine::read_body(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it, const int& idx, const char* buf, const ssize_t& nbytes)
{
	size_t body_size = reqs[idx].get_request_body_raw().size();
	size_t content_length = reqs[idx].get_content_length();
	if (content_length > meta_it->second.max_client_body || body_size > content_length)
	{
		Log::log("Corrupt size info, initiating early closing to avoid reading from socket buffer infinitely", WARNING);
		reqs[idx].flag_that_we_should_close_early();
		initiate_error_response(pfds_it, idx, CODE_413);
		return 1;
	}
	for (ssize_t i = 0; i < nbytes; i++)
		reqs[idx].append_byte_to_body(buf[i]);
	return 0;
}

/* Once it reads CRLF CRLF it begings parsing the headers and then reads body if applicable, then processes the request */
void ServerEngine::read_from_client_fd(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int		idx = meta_it->second.reqs_idx;
	char	buf[BUF_SZ + 1];
	ssize_t	nbytes;

	memset(buf, 0, BUF_SZ + 1);
	nbytes = recv(pfds_it->fd, buf, BUF_SZ, MSG_DONTWAIT);

	if (nbytes <= 0)
	{
		process_read_failure(pfds_it, meta_it, idx, nbytes);
		return;
	}
	if (reqs[idx].done_reading_headers())
	{
		if (read_body(pfds_it, meta_it, idx, buf, nbytes) != OK)
			return;
	}
	else
	{
		if (read_headers(pfds_it, idx, buf, nbytes) != OK)
			return;
	}
	update_client_activity_timestamp(meta_it);

	if (reqs[idx].done_reading_headers() && static_cast<size_t>(reqs[idx].get_request_body_raw().size()) == static_cast<size_t>(reqs[idx].get_content_length()))
	{
		try
		{
			process_request(pfds_it, idx);
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
	// if (reqs[idx].get_cgi_status() == EXECUTE)  // is CGI, but not done yet
	// 	return;
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
			std::ostringstream oss; oss << "send(): Client closed connection (EPIPE) or socket buffer full (EAGAIN) or EWOULDBLOCK, closing socket: " << pfds_it->fd << " and dropping the request.";
			Log::log(oss.str(), WARNING);
			// perror("send()");  // no way of logging errno through Log:: without calling errno explicitly
			reqs.erase(reqs.begin() + idx);
			forget_client(pfds_it, meta_it);  // Close the socket properly
		}
		else
		{
			reqs[idx].increment_total_sent_by(sz_to_send);
			update_client_activity_timestamp(meta_it);
		}
	}
	else  // sz_to_send == 0
	{
		std::ostringstream oss; oss << "Done writing to client on socket: " << pfds_it->fd;
		Log::log(oss.str(), DEBUG);
		if (reqs[idx].should_close_early())
		{
			Log::log("Comply with \'should_close_early\', forget client and their req", DEBUG);
			forget_client(pfds_it, meta_it);
			reqs.erase(reqs.begin() + idx);
		}
		else
			liberate_client_for_next_request(pfds_it, meta_it);
	}
}

/** (1) Close cgi_pipe_out
 * (2) Set response
 * (3) Locate client pfd to set to POLLOUT
*/
void ServerEngine::process_eof_on_pipe_out(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int idx = meta_it->second.reqs_idx;

	discard_cgi_pipe_out(pfds_it, meta_it);

	// simplistic CGI output inspection
	std::string headers = "";
	size_t headers_end_pos = reqs[idx].get_cgi_output().find("\r\n\r\n");
	size_t body_start_pos = 0;
	if (headers_end_pos != std::string::npos)
	{
		body_start_pos = headers_end_pos + 4;
		headers = reqs[idx].get_cgi_output().substr(0, headers_end_pos);
	}
	std::string body = reqs[idx].get_cgi_output().substr(body_start_pos);

	// set response
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
				 << headers << "\r\n"
				 << "Content-Length: " << body.length() << "\r\n"
				 << "\r\n"
				 << body;
	reqs[idx].set_response(response.str());

	// locate client pfd to set to POLLOUT
	for (std::map<int, pfd_info>::iterator it_met = pfd_info_map.begin(); it_met != pfd_info_map.end(); it_met++)
	{
		if (it_met->second.type == CLIENT_CONNECTION_SOCKET && it_met->second.reqs_idx == idx)
		{
			for (std::vector<pollfd>::iterator it_pfd = pfds.begin(); it_pfd != pfds.end(); it_pfd++)
			{
				if (it_pfd->fd == it_met->first)
				{
					it_pfd->events = POLLOUT;
					pfds_vector_modified = true;
					return;
				}
			}
		}
	}
}

void ServerEngine::process_unorderly_hangup(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	std::ostringstream oss; oss << "poll: socket " << pfds_it->fd << " hung up, unorderly shutdown";
	Log::log(oss.str(), DEBUG);
	forget_client(pfds_it, meta_it);
}

void ServerEngine::kill_cgi_process(const int& pipe_fd)
{
	for (std::map<int, pfd_info>::iterator it_met = pfd_info_map.begin(); it_met != pfd_info_map.end(); it_met++)
	{
		if (it_met->first == pipe_fd)
		{
			kill(it_met->second.cgi_pid, SIGTERM);
			it_met->second.cgi_pid = 0;
		}
	}
}

/* use pipe_fd to identify the pfd_it and the meta_it in order to initiate pfd termination, which also closes the pipe end */
void ServerEngine::locate_and_disable_cgi_pipe_pfd(const int& pipe_fd_to_kill)
{
	std::vector<pollfd>::iterator it_pfd;
	std::map<int, pfd_info>::iterator it_met;

	for (it_pfd = pfds.begin(); it_pfd != pfds.end(); it_pfd++)
	{
		if (it_pfd->fd == pipe_fd_to_kill)
			break;
	}
	for (it_met = pfd_info_map.begin(); it_met != pfd_info_map.end(); it_met++)
	{
		if (it_met->first == pipe_fd_to_kill)
			break;
	}
	terminate_pfd(it_pfd, it_met);
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

/** If
 * (1) is CLIENT_CONNECTION_SOCKET
 * (2) cgi_status == EXECUTE (bc NOT_CGI is not cgi, and status CGI_DONE is set after any proc has been killed and pipes closed to prevent it happening twice)
 * return true */
bool ServerEngine::is_client_and_cgi_timed_out(const pfd_info& pfd_meta)
{
	if (pfd_meta.reqs_idx == UNINITIALIZED)
		return false;

	if (pfd_meta.type == CLIENT_CONNECTION_SOCKET && reqs[pfd_meta.reqs_idx].get_cgi_status() == EXECUTE)
	{
		time_t now = time(NULL);
		long delta = now - pfd_meta.last_active;
		if (delta >= CGI_TIMEOUT / 1000)
			return true;
	}
	return false;
}

/**
 * If has had response, forget_client() silently, without a response
 * Else if connection without requests (telnet) -> 408
 */
void ServerEngine::process_connection_timeout(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	try
	{
		if (meta_it->second.has_had_response)  // silent close
		{
			// such a client should have reqs_idx == -1. But...what if...? .erase() just in case?
			if (meta_it->second.reqs_idx != UNINITIALIZED)
				Log::log("Found a stray request while silently closing a client!", ERROR);
			std::cout << "Client connection " << pfds_it->fd << " has timed out, closing the connection silently." << std::endl;
			forget_client(pfds_it, meta_it);
			return ;
		}
		else
		{
			initialize_new_request_if_no_active_one(meta_it);  // need one to store the response 408
			int idx = meta_it->second.reqs_idx;

			std::ostringstream oss; oss << "Client connection " << pfds_it->fd << " has timed out, responding 408.";
			Log::log(oss.str(), DEBUG);
			initiate_error_response(pfds_it, idx, CODE_408);
			reqs[idx].flag_that_we_should_close_early();
		}
	}
	catch (RequestException& e) {
		std::cout << e.what() << std::endl;
	}
}

void ServerEngine::process_cgi_timeout(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	int idx = meta_it->second.reqs_idx;

	if (reqs[idx].cgi->pipe_out[0] >= 0)
		kill_cgi_process(reqs[idx].cgi->pipe_out[0]);

	if (reqs[idx].cgi->pipe_in[1] >= 0)
	{
		locate_and_disable_cgi_pipe_pfd(reqs[idx].cgi->pipe_in[1]);
		reqs[idx].cgi->pipe_in[1] = UNINITIALIZED;
	}
	if (reqs[idx].cgi->pipe_out[0] >= 0)
	{
		locate_and_disable_cgi_pipe_pfd(reqs[idx].cgi->pipe_out[0]);
		reqs[idx].cgi->pipe_out[0] = UNINITIALIZED;
	}
	reqs[idx].set_cgi_status(CGI_DONE);
	reqs[idx].flag_that_we_should_close_early();
	initiate_error_response(pfds_it, idx, CODE_504);  // this would need more time per NGINX
}

/* Called as soon as Request reading has finished */
void ServerEngine::process_request(std::vector<pollfd>::iterator& pfds_it, const int& req_idx)
{
	RequestProcessor processor;
	std::string set_session_id;

	if (reqs[req_idx].get_cookies().empty())  // there was no Cookie header -> create session id
	{
		Log::log("-- Setting cookies for session -- ", WARNING);
		reqs[req_idx].set_cookies("sessionid=" + static_cast<std::ostringstream&>(std::ostringstream() << std::dec << time(NULL)).str());
		set_session_id = "Set-Cookie: sessionid=" + reqs[req_idx].get_cookies().begin()->second + "\r\n";
	}

	std::string response;
	response = processor.handleMethod(reqs[req_idx], server_blocks);

	if (reqs[req_idx].get_cgi_status() == EXECUTE)
	{
		try
		{
			if (reqs[req_idx].get_method() == GET)
			{
				// TODO handle CGI timeout
				reqs[req_idx].cgi->setup_cgi_get(pfds, pfd_info_map, req_idx);
				Log::log("CGI GET set up", DEBUG);
			}
			else
			{
				reqs[req_idx].cgi->setup_cgi_post(pfds, pfd_info_map, req_idx);
				Log::log("CGI POST set up", DEBUG);
			}

		} catch (CgiException & e)
		{
			std::ostringstream oss; oss << "CGI handling failed: " << e.what();
			Log::log(oss.str(), DEBUG);
			// TODO handle error
		}
		pfds_vector_modified = true;
	}
	else
	{
		if (!set_session_id.empty())
			response.insert(response.find("\r\n") + 2, set_session_id);
		reqs[req_idx].set_response(response);
		pfds_it->events = POLLOUT;  // get ready for writing, flag before potentially adding more pfds
	}
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

				// if (pfd_meta.type == LISTENER_SOCKET)
				// {
				// 	if (pfds_it->revents & POLLIN)
				// 		accept_client(pfds_it->fd, pfd_meta);
				// }
				// else if (pfd_meta.type == CGI_PIPE_OUT)  // will not have POLLIN if sleep(2), take it out of equation
				// {
				// 	if (pfds_it->revents & POLLIN)
				// 		read_from_cgi_pipe_out(pfds_it, meta_it);
				// }
				if (pfds_it->revents & POLLIN)
				{
					if (pfd_meta.type == LISTENER_SOCKET)
						accept_client(pfds_it->fd, pfd_meta);
					
					else if (pfd_meta.type == CGI_PIPE_OUT)
						read_from_cgi_pipe_out(pfds_it, meta_it);
					else
					{
						initialize_new_request_if_no_active_one(meta_it);
						read_from_client_fd(pfds_it, meta_it);  // has the try / catch
					}
				}
				else if (pfds_it->revents & POLLOUT)
				{
					if (pfd_meta.type == CGI_PIPE_IN)
						write_to_cgi_pipe_in(pfds_it, meta_it);
					else
						write_to_client(pfds_it, meta_it);
				}
				else if (pfds_it->revents & POLLHUP)
				{
					if (pfd_meta.type == CGI_PIPE_OUT)
						process_eof_on_pipe_out(pfds_it, meta_it);
					else
						process_unorderly_hangup(pfds_it, meta_it);
				}
				else if (pfds_it->revents & (POLLERR | POLLNVAL))
				{
					std::cout << "POLLERR | POLLNVAL" << std::endl;
					forget_client(pfds_it, meta_it);
				}
				else
				{
					if (is_client_and_timed_out(pfd_meta))
						process_connection_timeout(pfds_it, meta_it);
					else if (is_client_and_cgi_timed_out(pfd_meta))
						process_cgi_timeout(pfds_it, meta_it);
				}
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
	std::ostringstream oss; oss << "Closed " << i << " pfds before exiting";
	Log::log(oss.str(), DEBUG);
}
