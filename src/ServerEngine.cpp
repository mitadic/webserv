#include "ServerEngine.hpp"

ServerEngine::ServerEngine() {
		
	for (int i = 0; i < 10; i++)
		ports.push_back(9991 + i);
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

void ServerEngine::setup_listening_socket(int port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1 || !make_non_blocking(sockfd))
	{
		std::cerr << "Failed to create listening socket. Errno: " << errno << std::endl;
		return;
	}

	sockaddr_in socket_addr;
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = INADDR_ANY;
	socket_addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1)
	{
		std::cerr << "Failed to bind to port " << port << ". Errno:" << errno << std::endl;
		return;
	}
	if (listen(sockfd, 10) == -1)
	{
		std::cerr << "Failed to listen on socket. Errno: " << errno << std::endl;
		return;
	}

	pfd_info_map[sockfd] = (pfd_info){ .type = LISTENER_SOCKET, .sockaddr = socket_addr };  // map insertion

	std::cout << "Set up listener_fd no. " << sockfd << " for port no. " << port << std::endl;
}

void ServerEngine::init_pfds()
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
	reqs.push_back(Request());
	pfd_info_map[client] = (pfd_info){ .type = CLIENT_CONNECTION_SOCKET, .reqs_idx = reqs.size() - 1 };

	std::cout << "New client accepted on FD " << client << std::endl;
}

void ServerEngine::set_response(std::vector<pollfd>::iterator pfds_it, int idx)
{
	// parse request

	std::cout << "Finished reading the request: " << std::endl << "\"" << reqs[idx].request << "\"" << std::endl;
	if (reqs[idx].request.find(".py") != reqs[idx].request.npos) // if cgi request
	{
		reqs[idx].cgi.handle_cgi(pfds, pfd_info_map, idx);
		reqs[idx].cgi_status = READ_PIPE;
		reqs[idx].response = "HTTP/1.1 202 Accepted\nContent-Type: application/json\n\n{ \"job_id\": \"abc123\" }\n";
	}

	// else if request contains job_id and the job was finished
	else if (reqs[idx].request.find("job_id") != reqs[idx].request.npos)
	{
		// connect this client_id to that extant situation (request? handler? BIG QUESTION)
		// form response
	}
	else if (reqs[idx].request.find("about.html") != reqs[idx].request.npos)
	{
		reqs[idx].response = "HTTP/1.1 200\nContent-Type: text/html; charset=utf-8\n\n";

		std::ifstream	fSrc;
		fSrc.open("./www/html/about.html", std::ios::in);
		if (!fSrc.is_open()) {
			std::cerr << "Error opening file <" << "about.html" << ">" << std::endl;
			return;
		}

		std::string	line;
		while (std::getline(fSrc, line))
		{
			reqs[idx].response += line;
		}
		fSrc.close();
		if (fSrc.is_open())
			std::cerr << "Files not closed despite statements" << std::endl;
	}
	else if (reqs[idx].request.find("styles.css") != reqs[idx].request.npos)
	{
		reqs[idx].response = "HTTP/1.1 200\nContent-Type: text/css; charset=utf-8\n\n";

		std::ifstream	fSrc;
		fSrc.open("./www/html/css/styles.css", std::ios::in);
		if (!fSrc.is_open()) {
			std::cerr << "Error opening file <" << "styles.css" << ">" << std::endl;
			return;
		}

		std::string	line;
		while (std::getline(fSrc, line))
		{
			reqs[idx].response += line;
		}
		fSrc.close();
		if (fSrc.is_open())
			std::cerr << "Files not closed despite statements" << std::endl;
	}
	else  // ready to be sending basic HTML back
	// 	reqs[idx].response = "Hi. Default non-CGI response.$\n";
	
	pfds_it->events = POLLOUT;
}

void ServerEngine::run()
{
	std::signal(SIGINT, signal_handler); // handles Ctrl+C

	// Setup listening sockets for each port:
	for (size_t i = 0; i < ports.size(); i++)
		setup_listening_socket(ports[i]);

	// Initialize poll structure with the listening sockets, later adds client_fds
	init_pfds();

	while (!g_signal)
	{
		int timeout = CONNECTION_TIMEOUT;
		int events_count = poll(&pfds[0], pfds.size(), timeout);
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
		// std::cout << "pfds.size(): " << pfds.size() << std::endl;



		std::vector<pollfd>::iterator pfds_it = pfds.begin();
		while (!g_signal && pfds_it != pfds.end())
		{

			int									fd = pfds_it->fd;
			std::map<int, pfd_info>::iterator	meta_it = pfd_info_map.find(fd);
			struct pfd_info&					fd_meta = meta_it->second;
			int									idx = fd_meta.reqs_idx;

			if (fd_meta.type == LISTENER_SOCKET)
			{
				if (pfds_it->revents & POLLIN)
				{
					accept_client(fd, fd_meta);
					break;
				}
			}

			else if (pfds_it->revents & POLLIN)  // catch events on clients and CGI pipes : READ
			{
				char	buf[BUF_SZ];
				int		nbytes;

				memset(buf, 0, BUF_SZ);

				if (fd_meta.type == CGI_PIPE)
				{
					nbytes = read(fd, buf, BUF_SZ);  // this should never turn out zero when POLLIN
					if (nbytes < 0)
					{
						std::perror("read(pipe_fd)");
						// TODO: message the client
						close(fd);
						pfds.erase(pfds_it);
						pfd_info_map.erase(meta_it);
						break;  // will reset to pfds.begin()
					}
					reqs[idx].cgi_output.append(buf);
				}
				else  // is client, request reading to be done
				{
					nbytes = recv(fd, buf, BUF_SZ, MSG_DONTWAIT);
					if (nbytes <= 0)  // error or hangup
					{
						if (nbytes == 0)
							std::cout << "poll: socket " << pfds_it->fd << " hung up\n";
						else
							std::perror("recv");
						close(fd);
						pfds.erase(pfds_it);
						pfd_info_map.erase(meta_it);
						break;  // will reset to pfds.begin()
					}
					reqs[idx].request.append(buf);
					if (reqs[idx].request.find("\r\n\r\n") != reqs[idx].request.npos) // if (pretend) proper HTTP ending
					{
						std::cout << "Caught (pretend) proper HTTP ending" << std::endl;
						set_response(pfds_it, idx);
						break;
					}
				}
			}
			else if (pfds_it->revents & POLLOUT) // is established client : WRITE
			{
				size_t	sz_to_send = BUF_SZ;
				if (reqs[idx].response.size() - reqs[idx].total_sent < BUF_SZ)
					sz_to_send = reqs[idx].response.size() - reqs[idx].total_sent;

				if (sz_to_send)
				{
					send(fd, reqs[idx].response.substr(reqs[idx].total_sent).c_str(), sz_to_send, MSG_DONTWAIT);
					reqs[idx].total_sent += sz_to_send;
				}
				else
				{
					reqs[idx].reset();
					close(fd);
					pfds.erase(pfds_it);
					pfd_info_map.erase(meta_it);
					break;  // will reset to pfds.begin()
				}
			}
			else if (pfds_it->revents & POLLHUP)
			{
				// if cgi_pipe, EOF on pipe_fd
				if (fd_meta.type == CGI_PIPE)
				{
					close(fd);
					pfds.erase(pfds_it);
					// do not erase pfd_info_map, it's staying

					// formulate response 
					// reqs[idx].response = "We did the CGI, here it is: brrrrr\n";
					reqs[idx].cgi_status = AWAIT_CLIENT_RECONNECT;
					break;  // will reset to pfds.begin()
				}
				// else (is client or listener)
				else
				{
					std::cout << "poll: socket " << pfds_it->fd << " hung up\n";
					close(fd);
					pfds.erase(pfds_it);
					break;  // will reset to pfds.begin()
				}
			}
			else if (pfds_it->revents & (POLLERR | POLLNVAL))
			{
				std::cout << "POLLERR | POLLNVAL" << std::endl;
			}
			// else if (fd_meta.type == CLIENT_CONNECTION_SOCKET && !reqs[idx].request.empty())  // when recv() finishes, there's no flag
			// {
			// 	set_response(pfds_it, idx);  // setting response in invalid HTTP req, CONNECTION_TIMEOUT scenario
			// 	break;
			// }
			pfds_it++;
		}
	}
}