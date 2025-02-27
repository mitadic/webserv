#include "ServerEngine.hpp"

ServerEngine::ServerEngine() {
		
	for (int i = 0; i < 5; i++)
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

	pfd_info info = {};
	info.type = LISTENER_SOCKET;
	info.sockaddr = socket_addr;
	pfd_info_map[sockfd] = info;

	std::cout << "Set up listener_fd no. " << sockfd << " for port no. " << port << std::endl;
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
	pfd_info info = {};
	info.type = CLIENT_CONNECTION_SOCKET;
	info.reqs_idx = reqs.size() - 1;
	pfd_info_map[client] = info;

	std::cout << "New client accepted on FD " << client << std::endl;
}

void ServerEngine::set_response(std::vector<pollfd>::iterator& pfds_it, int idx)
{

	std::cout << "Finished reading the request: " << std::endl << "\"" << reqs[idx].request << "\"" << std::endl;
	if (reqs[idx].request.find(".py") != reqs[idx].request.npos) // if cgi request
	{
		reqs[idx].cgi.handle_cgi(pfds, pfd_info_map, idx);
		reqs[idx].cgi_status = READ_PIPE;
		reqs[idx].response = "HTTP/1.1 202 Accepted\nContent-Type: application/json\n\n{ \"job_id\": \"abc123\" }";
	}

	// else if request contains job_id and the job was finished
	else if (reqs[idx].request.find("job_id") != reqs[idx].request.npos)
	{
		// connect this client_id to that extant situation (request? handler? BIG QUESTION)
		// form response
	}
	else if (reqs[idx].request.find("about.html") != reqs[idx].request.npos)
	{
		reqs[idx].response = "HTTP/1.1 200\nContent-Type: text/html; charset=utf-8";

		std::ifstream	fSrc;
		fSrc.open("./www/three-socketeers/about.html", std::ios::in);
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
			std::cerr << "File not closed despite statements" << std::endl;
	}
	else if (reqs[idx].request.find("styles.css") != reqs[idx].request.npos)
	{
		reqs[idx].response = "HTTP/1.1 200\nContent-Type: text/css; charset=utf-8";

		std::ifstream	fSrc;
		fSrc.open("./www/three-socketeers/css/styles.css", std::ios::in);
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
			std::cerr << "File not closed despite statements" << std::endl;
	}
	else  // ready to be sending basic HTML back
	 	reqs[idx].response = "Hi. Default non-CGI response.$";
	
		 reqs[idx].response += "\r\n\r\n";
	reqs[idx].request.clear();
	pfds_it->events = POLLOUT;
}

void ServerEngine::set_basic_response(std::vector<pollfd>::iterator& pfds_it, int idx, std::string response)
{
	reqs[idx].response = response;
	reqs[idx].response += "\r\n\r\n";
	reqs[idx].request.clear();
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

void ServerEngine::run()
{
	std::signal(SIGINT, signal_handler); // handles Ctrl+C

	for (size_t i = 0; i < ports.size(); i++)
		setup_listening_socket(ports[i]);

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
					if (nbytes <= 0)  // hangup or error
					{
						if (nbytes == 0)
							std::cout << "poll: socket " << pfds_it->fd << " hung up, orderly shutdown\n";
						else
							std::perror("recv");
						if (close(fd) == -1)
							perror("close");
						pfds.erase(pfds_it);
						reqs.erase(reqs.begin() + idx);
						set_basic_response(pfds_it, idx, "HTTP/1.1 503 recv() fail");
						break;  // will reset to pfds.begin()
					}
					reqs[idx].request.append(buf);
					if (reqs[idx].request.find("\r\n\r\n") != reqs[idx].request.npos) // if proper HTTP ending
					{
						try {
							reqs[idx].parse();
						}
						catch (std::exception& e) {
							std::cerr << "Error: " << e.what() << std::endl;
							reqs[idx].response_status = std::atoi(e.what());
						}
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
					if (send(fd, reqs[idx].response.substr(reqs[idx].total_sent).c_str(), sz_to_send, MSG_DONTWAIT) == -1)
						perror("send");
					reqs[idx].total_sent += sz_to_send;
				}
				else  // sz_to_send == 0
				{
					if (reqs[idx].timed_out)
					{
						std::cout << "timeout: client on socket " << pfds_it->fd << ", closing connection\n";
						close(fd);
						pfds.erase(pfds_it);
						if (!reqs[idx].await_reconnection)
						{
							reqs.erase(reqs.begin() + idx);
							pfd_info_map.erase(meta_it);
						}
					}
					else
					{
						reqs[idx].reset();
						pfds_it->events = POLLIN;
					}
					break;  // will reset to pfds.begin()  --> ahh this was preventing other PFDs being reached!
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

					reqs[idx].cgi_status = AWAIT_CLIENT_RECONNECT;
					break;  // will reset to pfds.begin()
				}
				else
				{
					std::cout << "poll: socket " << pfds_it->fd << " hung up, unorderly shutdown\n";
					if (close(fd) == -1)
						perror("close");
					pfds.erase(pfds_it);
					reqs.erase(reqs.begin() + idx);
					break;  // will reset to pfds.begin()
				}
			}
			else if (pfds_it->revents & (POLLERR | POLLNVAL))
			{
				std::cout << "POLLERR | POLLNVAL" << std::endl;
			}
			else if (fd_meta.type == CLIENT_CONNECTION_SOCKET)  // no flag, so CONNECTION_TIMEOUT 
			{
				if (!reqs[idx].request.empty())
					set_basic_response(pfds_it, idx, "HTTP/1.1 400 Bad Request");
				else  // request is empty, we can close client connection
					set_basic_response(pfds_it, idx, "HTTP/1.1 408 Request Timeout");  // this would need more time
				reqs[idx].timed_out = true;
				break;
			}
			pfds_it++;
		}
	}
}