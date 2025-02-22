/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mitadic <mitadic@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 11:04:22 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/20 17:50:46 by mitadic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>		// close()
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
#include <poll.h>
#include <algorithm>
#include <csignal> // For signal handling
#include <cerrno> // For errno
#include <fcntl.h> // For fcntl
#include <cstring>


#include "CgiHandler.hpp"
#include "Request.hpp"

#define MAX_SERVER_BLOCKS 50
#define MAX_CONNECTIONS 500
#define CONNECTION_TIMEOUT 5000
#define BUF_SZ 2
#define OK 0

volatile std::sig_atomic_t g_signal = 0;

void signal_handler(int signal)
{
	g_signal = signal;
}

bool make_non_blocking(int &fd)
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

// Create the listening socket for all ports using IPv4 and TCP (Socket Stream) and store them in a map
// Create the address struct for all ports, bind and start listening (passive sockets)
void setup_listening_socket(int port, std::map<int, sockaddr_in>& map)
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

	map[sockfd] = socket_addr;
	std::cout << "Set up listener_fd no. " << sockfd << " for port no. " << port << std::endl;
}

void init_pfds(std::vector<struct pollfd> &pfds, std::map<int, sockaddr_in > &server_blocks)
{
	for (std::map<int, sockaddr_in>::iterator it = server_blocks.begin(); it != server_blocks.end(); it++)
	{
		struct pollfd fd;
		fd.fd = it->first;
		fd.events = POLLIN;
		if (pfds.size() >= MAX_SERVER_BLOCKS)
		{
			std::cerr << "Max connections reached." << std::endl;
			break;
		}
		pfds.push_back(fd);
	}
}

int accept_client(std::vector<struct pollfd> &pfds, std::map<int, sockaddr_in>::iterator &it)
{
	int addrlen = sizeof(it->second);
	int client = accept(it->first, (struct sockaddr*)&it->second, (socklen_t*)&addrlen);
	if (client == -1 || !make_non_blocking(client))
	{
		std::cerr << "Failed to grab connection. Errn: " << errno << std::endl;
		return (1);
	}
	struct pollfd fd;
	fd.fd = client;
	// std::cout << fd.fd << std::endl;
	fd.events = POLLIN;
	if (pfds.size() >= MAX_CONNECTIONS)
	{
		std::cerr << "Max connections reached." << std::endl;
		return (1);
	}
	pfds.push_back(fd);
	std::cout << "New client accepted on FD " << client << std::endl;
	return (OK);
}

int main()
{
	std::signal(SIGINT, signal_handler); // handles Ctrl+C
	std::vector<int> ports;  //

	// enable lookup "is this a server_block | cgi_pipe"
	std::map<int, sockaddr_in > server_blocks;	// pfd.fd <= sockaddr_in	| listener_fd to socketaddr_in
	std::map<int, pollfd> cgi_pipes;			// cli_fd <= pipe_fd[0]		| pipe_fd to client_fd
	std::vector<Request> reqs(MAX_CONNECTIONS); // pfd.fd <= reqs[pfd.fd]   | client_fd to req

	std::vector<struct pollfd> pfds;  //
	std::vector<CgiHandler> cgi_objects;

	ports.push_back(9991);  //
	ports.push_back(9992);  //
	ports.push_back(9993);  //

	// Setup listening sockets for each port:
	for (size_t i = 0; i < ports.size(); i++)
		setup_listening_socket(ports[i], server_blocks);

	// Initialize poll structure with the listening sockets, later adds client_fds
	init_pfds(pfds, server_blocks);

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
		// else if (events_count == 0)
		// 	continue;
		std::cout << "pfds.size(): " << pfds.size() << std::endl;
		std::vector<pollfd>::iterator pfds_it = pfds.begin();
		while (!g_signal && pfds_it != pfds.end())
		{
			int fd = pfds_it->fd;

			std::map<int, sockaddr_in>::iterator sb_it = server_blocks.find(fd);
			if (sb_it != server_blocks.end()) // is one of the listeners
			{
				if (pfds_it->revents & POLLIN)
				{
					if (accept_client(pfds, sb_it) != OK)
						continue;
					reqs[fd].client_fd = fd;
				}
			}

			else if (pfds_it->revents & POLLIN)  // catch events on clients and CGI pipes : READ
			{
				char buf[BUF_SZ];
				int nbytes;
				memset(buf, 0, BUF_SZ);

				std::map<int, pollfd>::iterator cgi_pipes_it = cgi_pipes.find(fd);
				if (cgi_pipes_it != cgi_pipes.end())
				{
					nbytes = read(fd, buf, BUF_SZ);  // this should never turn out zero when POLLIN
					std::cout << "read "<< nbytes << " bytes from cgi_pipe: " << buf << std::endl;
					if (nbytes < 0)
					{
						std::perror("read(pipe_fd)");
						close(fd);
						// deal with the client pfd too
						pfds.erase(pfds_it);
						break;  // will reset to pfds.begin()
					}
					reqs[cgi_pipes_it->second.fd].cgi_output.append(buf);
				}
				else  // is client, request reading to be done
				{
					nbytes = recv(fd, buf, BUF_SZ, MSG_DONTWAIT);
					std::cout << "read "<< nbytes << " bytes from request: " << buf << std::endl;
					if (nbytes <= 0)  // error or hangup
					{
						if (nbytes == 0)
							std::cout << "poll: socket " << pfds_it->fd << " hung up\n";
						else
							std::perror("recv");
						close(fd);
						pfds.erase(pfds_it);
						break;  // will reset to pfds.begin()
					}
					reqs[fd].request.append(buf);
				}
			}
			else if (pfds_it->revents & POLLOUT) // is established client : WRITE
			{
				size_t	sz_to_send = BUF_SZ;
				if (reqs[fd].response.substr(reqs[fd].total_sent).size() < BUF_SZ)
					sz_to_send = reqs[fd].response.size() - reqs[fd].response.substr(reqs[fd].total_sent).size();

				if (sz_to_send)
					send(fd, reqs[fd].response.substr(reqs[fd].total_sent).c_str(), sz_to_send, MSG_DONTWAIT);
				else
				{
					close(fd);
					pfds.erase(pfds_it);
					break;  // will reset to pfds.begin()
				}
			}
			else if (pfds_it->revents & POLLHUP)
			{
				// if cgi_pipe, EOF on pipe_fd
				std::map<int, pollfd>::iterator cgi_pipes_it = cgi_pipes.find(fd);
				if (cgi_pipes_it != cgi_pipes.end())
				{
					close(fd);
					pfds.erase(pfds_it);
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
			else  // when recv() finishes, there's no flag
			{
				if (reqs[fd].request.empty()) // no revent and empty request? Can this even happen?
				{
					// std::cout << "no flag on revents, and request is empty" << std::endl;
					continue;
				}

				// parse request

				std::cout << "Finished reading the request: " << std::endl << "\"" << reqs[fd].request << "\"" << std::endl;
				if (reqs[fd].request.find(".py") != reqs[fd].request.npos) // if cgi request
				{
					reqs[fd].cgi.handle_cgi(pfds, cgi_pipes);
					reqs[fd].response = "HTTP/1.1 202 Accepted\nContent-Type: application/json\n\n{ \"job_id\": \"abc123\" }\n";
				}
				// else if request contains job_id and the job was finished
					// connect this client_id to that extant situation (request? handler? BIG QUESTION)
					// form response
				else  // ready to be sending basic HTML back
					reqs[fd].response = "Hi. Default non-CGI response";
				
				pfds_it->events = POLLOUT;
			}
			pfds_it++;
		}
	}
	//cleanup
	for (std::vector<struct pollfd>::iterator it = pfds.begin(); it != pfds.end(); it++)
		close(it->fd);
}


// Close connections



