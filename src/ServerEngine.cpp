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
		Log::log("Max connections reached", ERROR);
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
	info.request = NULL;
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
	if (pfds_it->fd != UNINITIALIZED)
	{
		if (close(pfds_it->fd) == -1)
			perror("close");
	}
	pfds.erase(pfds_it);
	pfd_info_map.erase(meta_it);
	pfds_vector_modified = true;
}

void ServerEngine::forget_client(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	std::ostringstream oss; oss << "Forgetting client on socket FD: " << pfds_it->fd;
	Log::log(oss.str(), DEBUG);
	delete meta_it->second.request;
	meta_it->second.request = NULL;
	terminate_pfd(pfds_it, meta_it);
}

/* terminate_pfd associated with CGI_PIPE_IN and set cgi's pipe_in[1] to -1 */
void ServerEngine::discard_cgi_pipe_in(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	Request *request = meta_it->second.request;

	std::ostringstream oss; oss << "Discarding cgi_pipe_in with FD: " << pfds_it->fd;
	Log::log(oss.str(), INFO);
	terminate_pfd(pfds_it, meta_it);
	request->cgi->pipe_in[1] = UNINITIALIZED;
}

/* terminate_pfd associated with CGI_PIPE_OUT and set cgi's pipe_out[0] to -1 */
void ServerEngine::discard_cgi_pipe_out(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	Request *request = meta_it->second.request;

	std::ostringstream oss; oss << "Discarding cgi_pipe_out with FD: " << pfds_it->fd;
	Log::log(oss.str(), INFO);
	terminate_pfd(pfds_it, meta_it);
	request->cgi->pipe_out[0] = UNINITIALIZED;
}

/**
 * Initiate error response
 * @param pfds_it of the client socket to be set to POLLOUT
 * @param idx of the Request to set error response to
 * @param code CODE_XYZ
 */
void ServerEngine::initiate_error_response(std::vector<pollfd>::iterator& pfds_it, Request *request, int code)
{
	request->set_response_status(code);
	request->set_response(ErrorPageGenerator::createErrorPage(*request, server_blocks));
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
		if (pipe_out_meta_it->second.type == CGI_PIPE_OUT && pipe_out_meta_it->second.request == meta_it->second.request)  // found the cgi_pipe_out
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

/** Process write() returned -1:
 * (1) locate related client_pfd_it and use to initiate_error_response for related Request
 * (2) throw away cgi proc and pipes
 */
void ServerEngine::process_write_failure(Request *request)
{
	Log::log("write() to cgi_pipe_in", ERROR);
	throw_away_cgi_proc_and_pipes(request);

	std::vector<pollfd>::iterator client_pfd_it;
	for (client_pfd_it = pfds.begin(); client_pfd_it != pfds.end(); client_pfd_it++)
	{
		if (client_pfd_it->fd == request->get_client_fd())
			break;
	}
	initiate_error_response(client_pfd_it, request, CODE_500);
}

void ServerEngine::write_to_cgi_pipe_in(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	size_t sz_to_send = BUF_SZ;
	Request *request = meta_it->second.request;
	int body_size = request->get_content_length();
	int sent_so_far = request->get_total_sent();

	if (body_size - sent_so_far < BUF_SZ)
	{
		if (body_size - sent_so_far < 0)
			std::cerr << "Critical error identified write()-ing to cgi_pipe_in: sz_to_send less than 0. sz_to_send: " << sz_to_send << ", body_size: " << body_size << std::endl;
		sz_to_send = body_size - sent_so_far;
	}

	if (sz_to_send)
	{
		if (write(pfds_it->fd, &(request->get_request_body_raw().data())[sent_so_far], sz_to_send) == -1)
		{
			process_write_failure(request);
			return;
		}
		// std::cout << "DEBUG: successfully written to cgi_pipe_in: " << str_to_send.data() << std::endl;
		request->increment_total_sent_by(sz_to_send);
		update_client_activity_timestamp(meta_it);
	}
	else  // sz_to_send == 0
	{
		request->set_total_sent(0);  // reset to reuse same attribute for when writing to client_fd
		// set_matching_cgi_pipe_out_to_pollin(meta_it);  // UPDATE: is again already set from the get go, rm this
		discard_cgi_pipe_in(pfds_it, meta_it);
	}
}

/** Process read() returned -1:
 * (1) locate related client_pfd_it and use to initiate_error_response for related Request
 * (2) throw away cgi proc and pipe(s) -- in this case it will only be the CGI_PIPE_OUT
 */
void ServerEngine::process_read_failure(Request *request)
{
	Log::log("read() from cgi_pipe_out", ERROR);
	throw_away_cgi_proc_and_pipes(request);

	std::vector<pollfd>::iterator client_pfd_it;
	for (client_pfd_it = pfds.begin(); client_pfd_it != pfds.end(); client_pfd_it++)
	{
		if (client_pfd_it->fd == request->get_client_fd())
			break;
	}
	initiate_error_response(client_pfd_it, request, CODE_500);
}

/* read() should never turn out zero when POLLIN, we rely on POLLHUP to catch EOF */
void ServerEngine::read_from_cgi_pipe_out(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	char	buf[BUF_SZ + 1];
	ssize_t	nbytes;

	memset(buf, 0, BUF_SZ + 1);
	nbytes = read(pfds_it->fd, buf, BUF_SZ);
	if (nbytes < 0)
	{
		process_read_failure(meta_it->second.request);
		return;
	}
	meta_it->second.request->append_to_cgi_output(buf);
}

/* Use client's meta information (host, port, fd) to do 'new Request' */
void ServerEngine::initialize_new_request_if_no_active_one(std::map<int, pfd_info>::iterator& meta_it)
{
	if (meta_it->second.request == NULL)
	{
		std::ostringstream oss; oss << "Initializing new request for client_fd [" << meta_it->first << "] since no active one";
		Log::log(oss.str(), DEBUG);
		meta_it->second.request = new Request(meta_it->second.host, meta_it->second.port, meta_it->first);
		if (meta_it->second.request == NULL)
			Log::log("Malloc fail for Request, this is bad, mkay", ERROR);
	}
}

/* Does not rm the client's meta, just resets the reqs_idx to UNINITIALIZED */
void ServerEngine::liberate_client_for_next_request(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	delete meta_it->second.request;
	meta_it->second.request = NULL;
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
void ServerEngine::process_recv_failure(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it, Request *request, const ssize_t& nbytes)
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
		std::ostringstream oss; oss << "recv: " << std::strerror(errno);
		Log::log(oss.str(), ERROR);
		initiate_error_response(pfds_it, request, CODE_503);
	}
}

/* Read headers, add any buf spillover to body, and parse and validate the headers before proceeding */
int ServerEngine::read_headers(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it, Request* request, const char* buf, const ssize_t& nbytes)
{
	ssize_t the_trail_from_prev_sz = 0;
	if (request->get_request_str().size() >= 3)
		the_trail_from_prev_sz = 3;
	size_t tail_start = request->get_request_str().size() - the_trail_from_prev_sz;

	// this_buffer == 3 tailing chars of request_str (if applicable) + current buf, to catch broken CRLFCRLF
	std::string this_buffer = request->get_request_str().substr(tail_start); // if no tail, will be ""
	this_buffer += buf;
	size_t crlf_begin = this_buffer.find("\r\n\r\n");
	if (crlf_begin == std::string::npos)
	{
		request->append_to_request_str(buf); // pure buf without consideration for trail
	}
	else
	{
		request->switch_to_reading_body();
		std::string buf_as_str(this_buffer.c_str() + 0, this_buffer.c_str() + crlf_begin + 2);
		request->append_to_request_str(buf_as_str);

		try {
			request->parse();
			// is there a cgi? -> add it to pfds
		}
		catch (RequestException& e) {
			Log::log("Corrupt headers, initiating early closing to prevent processing unknown socket buffer", WARNING);
			request->flag_that_we_should_close_early();
			initiate_error_response(pfds_it, request, e.code());
			return 1;
		}

		// do trailing body this round of POLLIN
		// read_body(pfds_it, meta_it, idx, (&this_buffer.c_str()[crlf_begin + 4]), nbytes + the_trail_from_prev_sz - (crlf_begin + 4));
		read_body(pfds_it, meta_it, (&buf[0 - the_trail_from_prev_sz + crlf_begin + 4]), nbytes + the_trail_from_prev_sz - (crlf_begin + 4));
	}
	return 0;
}

/** @param pfds_it client
 * @param meta_it client meta
 * @param idx of reqs, shorthand, also available in client meta
 * @param buf c_str() and not necessarily 0-terminated
 * @param nbytes size of buf
 * Read body */
int ServerEngine::read_body(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it, const char* buf, const ssize_t& nbytes)
{
	Request *request = meta_it->second.request;
	// if not chunked, read with Content-Length
	if (request->is_flagged_as_chunked() == false)
	{
		size_t body_size = request->get_request_body_raw().size();
		size_t content_length = request->get_content_length();
		if (content_length > meta_it->second.max_client_body || body_size > content_length)
		{
			Log::log("Disallowed size (or corrupt Content-Length info), initiating early closing to avoid reading from socket buffer infinitely", WARNING);
			request->flag_that_we_should_close_early();
			initiate_error_response(pfds_it, request, CODE_413);
			return 1;
		}
		for (ssize_t i = 0; i < nbytes; i++)
			request->append_byte_to_body(buf[i]);
		return 0;
	}

	// else read as chunked
	int i = 0;
	while (i < nbytes)
	{
		unsigned long res = 0;

		// if needing new chunk_size (also the case if hex_str has stuff from prev BUF but no delimiter yet)
		while (request->get_chunk_size() == UNINITIALIZED && i < nbytes)
		{
			request->set_chunk_size_hex_str(request->get_chunk_size_hex_str() + buf[i]);
			if (request->get_chunk_size_hex_str().size() >= 2)
			{
				std::string tail_two = request->get_chunk_size_hex_str().substr(request->get_chunk_size_hex_str().size() - 2);
				if (tail_two.find("\r\n") != std::string::npos)
				{
					std::istringstream s(request->get_chunk_size_hex_str().substr(0, request->get_chunk_size_hex_str().size() - 2));  // ditch delimiters
					s >> std::hex >> res;
					if (s.fail())
					{
						Log::log("Corrupt chunk_size, initiating early closing to avoid reading from socket buffer infinitely", WARNING);
						request->flag_that_we_should_close_early();
						initiate_error_response(pfds_it, request, CODE_400);
						return 1;
					}
					if (res == 0)
					{
						request->flag_that_done_reading_chunked_body();
						return OK;
					}
					request->set_chunk_size(res);
					request->set_nread_of_chunk_size(0);
					request->set_chunk_size_hex_str("");
				}
			}
			i++;
		}
		
		// read the chunk
		while (request->get_nread_of_chunk_size() < request->get_chunk_size() && i < nbytes)
		{
			request->append_byte_to_body(buf[i]);
			request->set_nread_of_chunk_size(request->get_nread_of_chunk_size() + 1);
			i++;
		}

		// just finished reading a chunk
		if (request->get_nread_of_chunk_size() != UNINITIALIZED && request->get_nread_of_chunk_size() == request->get_chunk_size() && i < nbytes)
		{
			// bypass delimiter if present
			if (buf[i] == '\r')
			{
				i++;
				if (i < nbytes && buf[i] == '\n')
					i++;
			}
			else if (i == 0 && buf[i] == '\n')
				i++;
			else
			{
				Log::log("Bad chunk delimiter, initiating early closing to avoid reading from socket buffer infinitely", WARNING);
				request->flag_that_we_should_close_early();
				initiate_error_response(pfds_it, request, CODE_400);
				return 1;
			}
			// reset
			request->set_chunk_size(UNINITIALIZED);
		}

		// went past max_body
		if (request->get_request_body_raw().size() > meta_it->second.max_client_body)
		{
			Log::log("Chunked body too large, initiating early closing to avoid reading from socket buffer infinitely", WARNING);
			request->flag_that_we_should_close_early();
			initiate_error_response(pfds_it, request, CODE_413);
			return 1;
		}
	}
	return OK;
}

/* Once it reads CRLF CRLF it begings parsing the headers and then reads body if applicable, then processes the request */
void ServerEngine::read_from_client_fd(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	char	buf[BUF_SZ + 1];
	ssize_t	nbytes;
	Request *request = meta_it->second.request;

	memset(buf, 0, BUF_SZ + 1);
	nbytes = recv(pfds_it->fd, buf, BUF_SZ, MSG_DONTWAIT);

	if (nbytes <= 0)
	{
		process_recv_failure(pfds_it, meta_it, request, nbytes);
		return;
	}
	if (request->done_reading_headers())
	{
		if (read_body(pfds_it, meta_it, buf, nbytes) != OK)
			return;
	}
	else
	{
		if (read_headers(pfds_it, meta_it, request, buf, nbytes) != OK)
			return;
	}
	update_client_activity_timestamp(meta_it);

	if ((request->done_reading_headers() && static_cast<size_t>(request->get_request_body_raw().size()) == static_cast<size_t>(request->get_content_length()))
		|| (request->done_reading_chunked_body()))
	{
		try
		{
			process_request(pfds_it, request);
		}
		catch (std::exception& e)
		{
			Log::log(e.what(), ERROR);
			if (dynamic_cast<RequestException*>(&e))
				initiate_error_response(pfds_it, request, dynamic_cast<RequestException*>(&e)->code());
			else
				initiate_error_response(pfds_it, request, CODE_500);
		}
	}
}

void ServerEngine::write_to_client(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	size_t sz_to_send = BUF_SZ;
	Request *request = meta_it->second.request;

	// TODO: this check might be obsolete / superfluous
	if (request == NULL)
	{
		forget_client(pfds_it, meta_it);
		return;
	}
	int response_size = request->get_response().size();
	int sent_so_far = request->get_total_sent();

	if (response_size - sent_so_far < BUF_SZ) {
		if (response_size - sent_so_far < 0)
			std::cerr << "Critical error identified: sz_to_send less than 0" << std::endl;
		sz_to_send = response_size - sent_so_far;
	}

	if (sz_to_send)
	{
		std::string str_to_send = request->get_response().substr(request->get_total_sent());
		if (send(pfds_it->fd, str_to_send.c_str(), sz_to_send, MSG_DONTWAIT) == -1)
		{
			std::ostringstream oss; oss << "send(): Client closed connection (EPIPE) or socket buffer full (EAGAIN) or EWOULDBLOCK, closing socket: " << pfds_it->fd << ", and dropping the request.";
			Log::log(oss.str(), WARNING);
			forget_client(pfds_it, meta_it);
			return;
		}
		request->increment_total_sent_by(sz_to_send);
		update_client_activity_timestamp(meta_it);
	}
	else  // sz_to_send == 0
	{
		std::ostringstream oss; oss << "Done writing to client on socket: " << pfds_it->fd;
		Log::log(oss.str(), DEBUG);
		if (request->should_close_early() || !request->should_keep_alive())
		{
			Log::log("Comply with \'should_close_early\' or with \'Connection: close\', erase req and forget client", DEBUG);
			forget_client(pfds_it, meta_it);
		}
		else
			liberate_client_for_next_request(pfds_it, meta_it);
	}
}

/* Called only after POLLHUP on CGI_PIPE_OUT, includes parsing */
void ServerEngine::process_raw_cgi_output(Request *request)
{
	CgiResponse cgi_response;
	cgi_response.parse_raw_cgi_output(request->get_cgi_output());
	request->set_response(cgi_response.get_formatted_response());
}

/* Has the sole waitpid() call */
void check_in_on_subprocess(const pid_t pid)
{
	pid_t w;
	int wstatus;

	// wait to inspect for inconsistencies or errors
	w = waitpid(pid, &wstatus, WNOHANG);
	if (w <= 0)
	{
		std::ostringstream oss; oss << "waitpid: " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		throw RequestException(CODE_500);
	}
	else if (WEXITSTATUS(wstatus) != 0)
	{
		Log::log("CGI subprocess exited with a non-0 value, reporting 500", WARNING);
		throw RequestException(CODE_500);
	}
}

/**
 * (1) Close and discard cgi_pipe_out
 * (2) Set response: includes cgi_output parsing and necessary adaptions such as that of Content-Length
 * (3) Locate client pfd to set to POLLOUT
*/
void ServerEngine::process_eof_on_pipe_out(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	Request *request = meta_it->second.request;
	const pid_t pid = meta_it->second.cgi_pid;

	discard_cgi_pipe_out(pfds_it, meta_it);
	try
	{
		check_in_on_subprocess(pid);
		std::ostringstream oss; oss << "Raw CGI output:\n******\n" << request->get_cgi_output() << "\n******";
		Log::log(oss.str(), SETUP);
		process_raw_cgi_output(request);
	}
	catch (RequestException& e)
	{
		request->set_response_status(e.code());
		request->set_response(ErrorPageGenerator::createErrorPage(*request, server_blocks));
	}

	std::ostringstream oss; oss << "Response to the CGI request:\n******\n" << request->get_response() << "\n******";
	Log::log(oss.str(), SETUP);

	// locate client pfd to set to POLLOUT
	for (std::map<int, pfd_info>::iterator it_met = pfd_info_map.begin(); it_met != pfd_info_map.end(); it_met++)
	{
		if (it_met->second.type == CLIENT_CONNECTION_SOCKET && it_met->second.request == request)
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

/* This doesn't have to be O(n) though, it can be O(logn) via idx lookup */
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

/** use pipe_fd to identify the pfd_it and the meta_it in order to initiate pfd termination, which also closes the pipe end
 * @param pipe_fd_to_kill fd used to O(n) identify the pfd and the meta to remove
*/
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
	if (it_met->second.type == CGI_PIPE_IN)
		discard_cgi_pipe_in(it_pfd, it_met);
	else if (it_met->second.type == CGI_PIPE_OUT)
		discard_cgi_pipe_out(it_pfd, it_met);
	// terminate_pfd(it_pfd, it_met);  // better to use the "discard" ones, they give the useful identity printouts
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
 * (2) cgi_status == EXECUTE (bc NOT_CGI is not cgi, and status CGI_DONE is set after any proc has been killed and pipes closed to prevent the timeout TRUE being triggered more than once)
 * return true */
bool ServerEngine::is_client_and_cgi_timed_out(const pfd_info& pfd_meta)
{
	if (pfd_meta.request == NULL)
		return false;

	if (pfd_meta.type == CLIENT_CONNECTION_SOCKET && pfd_meta.request->get_cgi_status() == EXECUTE)
	{
		time_t now = time(NULL);
		long delta = now - pfd_meta.last_active;
		if (delta >= CGI_TIMEOUT / 1000)
			return true;
	}
	return false;
}

/**
 * If has_had_response, forget_client() silently, without a response
 * Else if connection without requests (telnet) -> 408
 */
void ServerEngine::process_connection_timeout(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	try
	{
		if (meta_it->second.has_had_response)  // silent close
		{
			std::ostringstream oss; oss << "Client connection " << pfds_it->fd << " has timed out, closing the connection silently.";
			Log::log(oss.str(), DEBUG);
			forget_client(pfds_it, meta_it);
		}
		else
		{
			initialize_new_request_if_no_active_one(meta_it);  // need one to store the response 408

			std::ostringstream oss; oss << "Client connection " << pfds_it->fd << " has timed out without sending a request, responding 408.";
			Log::log(oss.str(), DEBUG);
			initiate_error_response(pfds_it, meta_it->second.request, CODE_408);
			meta_it->second.request->flag_that_we_should_close_early();
		}
	}
	catch (RequestException& e) {
		std::cout << e.what() << std::endl;
	}
}

/** Called both by CLIENT_CONNECTION_SOCKET (if timeout) and CGI_PIPE_IN / _OUT (if write/read fail), so yes, CGI_PIPE_IN / _OUT end up passing reqs_idx only to enable O(n) looking for themselves
 * (1) kill cgi process
 * (2) if CGI_PIPE_IN, disable
 * (3) if CGI_PIPE_OUT, disable
*/
void ServerEngine::throw_away_cgi_proc_and_pipes(Request *request)
{
	if (request->cgi->pipe_out[0] >= 0)
		kill_cgi_process(request->cgi->pipe_out[0]);

	if (request->cgi->pipe_in[1] >= 0)
	{
		locate_and_disable_cgi_pipe_pfd(request->cgi->pipe_in[1]);
		request->cgi->pipe_in[1] = UNINITIALIZED;
	}
	if (request->cgi->pipe_out[0] >= 0)
	{
		locate_and_disable_cgi_pipe_pfd(request->cgi->pipe_out[0]);
		request->cgi->pipe_out[0] = UNINITIALIZED;
	}
}

/* Only called for meta_it.type == CLIENT_CONNECTION_SOCKET */
void ServerEngine::process_cgi_timeout(std::vector<pollfd>::iterator& pfds_it, std::map<int, pfd_info>::iterator& meta_it)
{
	Request *request = meta_it->second.request;

	throw_away_cgi_proc_and_pipes(request);
	request->set_cgi_status(CGI_DONE);
	request->flag_that_we_should_close_early();
	request->set_total_sent(0);  // reset for reuse, was used by write() but got interrupted, not to be used by send()
	initiate_error_response(pfds_it, request, CODE_504);  // this would need more time per NGINX
}

void ServerEngine::normalize_uri(std::vector<ServerBlock>& server_blocks, Request *request)
{
	// normalize URI with consideration of the location block
	std::string originalUri = request->get_request_uri();
	const Location *matchingLocation1 = Utils::getLocation(*request, Utils::getServerBlock(*request, server_blocks));
	request->set_request_uri(originalUri + "/");
	const Location *matchingLocation2 = Utils::getLocation(*request, Utils::getServerBlock(*request, server_blocks));
	if (matchingLocation1 && matchingLocation2)
	{
		if (matchingLocation1->get_path() == matchingLocation2->get_path())
			request->set_request_uri(originalUri);
	}
}

/* Called as soon as Request reading has finished */
void ServerEngine::process_request(std::vector<pollfd>::iterator& pfds_it, Request *request)
{
	RequestProcessor processor;
	std::string set_session_id;

	std::ostringstream oss; oss << "Raw Request:\n******\n" << request->get_request_str() << "\r\n" << request->get_request_body_as_str() << "\n******";
	Log::log(oss.str(), SETUP);

	if (request->get_cookies().empty())  // there was no Cookie header -> create session id
	{
		Log::log("-- Setting cookies for session -- ", WARNING);
		request->set_cookies("sessionid=" + static_cast<std::ostringstream&>(std::ostringstream() << std::dec << time(NULL)).str());
		set_session_id = "Set-Cookie: sessionid=" + request->get_cookies().begin()->second + "\r\n";
	}

	normalize_uri(server_blocks, request);

	std::string response;
	response = processor.handleMethod(*request, server_blocks);

	if (request->get_cgi_status() == EXECUTE)
	{
		try
		{
			if (request->get_method() == GET)
			{
				request->cgi->setup_cgi_get(pfds, pfd_info_map, request);
				Log::log("CGI GET set up", DEBUG);
			}
			else if (request->get_method() == POST)
			{
				request->cgi->setup_cgi_post(pfds, pfd_info_map, request);
				Log::log("CGI POST set up", DEBUG);
			}
			else  // should already be caught in detect_cgi(), but jic
			{
				std::ostringstream oss; oss << "CGI method " << request->get_method() << " unimplemented, responding 501";
				Log::log(oss.str(), WARNING);
				throw RequestException(CODE_501);
			}

		}
		catch (std::exception& e)
		{
			std::ostringstream oss; oss << "CGI setup failed: " << e.what();
			Log::log(oss.str(), WARNING);
			if (dynamic_cast<RequestException*>(&e))
				initiate_error_response(pfds_it, request, dynamic_cast<RequestException*>(&e)->code());
			else
				initiate_error_response(pfds_it, request, CODE_500);  // less than ideal, CgiException is just a blueprint currently
		}
		pfds_vector_modified = true;
	}
	else
	{
		if (!set_session_id.empty())
			response.insert(response.find("\r\n") + 2, set_session_id);
		request->set_response(response);
		std::ostringstream oss; oss << "Response:\n******\n" << request->get_response() << "\n******";
		Log::log(oss.str(), SETUP);
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
			std::ostringstream oss; oss << "Erasing server block at index " << i << std::endl;
			Log::log(oss.str(), DEBUG);
			failed_indexes.pop_back();
		}
		i--;
	}
}

void ServerEngine::run()
{
	std::signal(SIGINT, signal_handler); // handles Ctrl+C
	std::signal(SIGTERM, signal_handler); // handles kill -15
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
		int events_count = poll(&pfds[0], pfds.size(), CGI_TIMEOUT);
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
					// can this only occur for client_fd tho?
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
				Log::log("Inside top-level Engine catch block, shouldn't occur", ERROR);
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
