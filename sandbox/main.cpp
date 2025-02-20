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
#include <sys/wait.h> // For waitpid()
#include <cstring>


#include "CgiHandler.hpp"
#include "Request.hpp"

#define MAX_SERVER_BLOCKS 50
#define MAX_CONNECTIONS 500
#define CONNECTION_TIMEOUT 5000
#define BUFF_SZ 256

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
		std::cout << "Failed to create listening socket. Errno: " << errno << std::endl;
		exit(EXIT_FAILURE);
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
		std::cout << "Failed to grab connection. Errn: " << errno << std::endl;
		return (1);
	}
	struct pollfd fd;
	fd.fd = client;
	// std::cout << fd.fd << std::endl;
	fd.events = POLLIN;
	if (pfds.size() >= MAX_CONNECTIONS)
	{
		std::cout << "Max connections reached." << std::endl;
		return (1);
	}
	pfds.push_back(fd);
	return (0);
}

void handle_cgi(std::vector<struct pollfd> &pfds, CgiHandler &cgi, std::vector<int> pipe_ends_for_cgi)
{
	if (pipe(cgi.pipe) < 0)
	{
		std::perror("pipe");
		return;
	}

	pid_t pid = fork(); //create child
	if (pid < 0)
	{
		std::perror("fork");
		return;
	}

	if (pid == 0)
	{
		close(cgi.pipe[0]);  // close the end not used in child right away

		dup2(cgi.pipe[1], STDOUT_FILENO);
		close(cgi.pipe[1]);
		execve("/usr/bin/python3", cgi.argv, NULL);
		std::perror("execve");
		return;
	}
	else
	{
		pid_t w;
		int wstatus;

		if (close(cgi.pipe[1]) < 0)  // close the end used in child before waiting on child
		{
			std::perror("close");
			return;
		}
		w = waitpid(pid, &wstatus, 0);
		if (w < 0)
		{
			std::perror("waitpid");
			return; // placeholder
		}

		// char readbuff[BUFF_SZ];
		// ssize_t bytes_read = 0;
		// while ((bytes_read = read(cgi.pipe[0], readbuff, BUFF_SZ)) > 0)
		// {
		// 	cgi.response.append(readbuff);
		// 	readbuff[bytes_read] = 0;
		// 	std::cout << "readbuff: " << readbuff << std::endl;
		// }
		// std::cout << "CGI response: " << cgi.response << std::endl;
		// close(cgi.pipe[0]);


		// place cgi.pipe[0] both into pfds, and into pipe_ends_for_cgi
		struct pollfd fd;
		fd.fd = cgi.pipe[0]; // read end (bc we read)
		pipe_ends_for_cgi.push_back(cgi.pipe[0]);
		fd.events = POLLIN;
		pfds.push_back(fd);

	}
}

int main()
{
	std::signal(SIGINT, signal_handler); // handles Ctrl+C
	std::vector<int> ports;  //
	std::map<int, sockaddr_in > server_blocks;  //
	// std::vector<int> pipe_ends_for_cgi;
	std::vector<struct pollfd> pfds;  //
	std::vector<CgiHandler> cgi_objects;
	// CgiHandler cgi;  // now default constructor disallowed
	std::vector<Request> reqs(MAX_CONNECTIONS);

	ports.push_back(9991);  //
	ports.push_back(9992);  //
	ports.push_back(9993);  //

	// Setup listening sockets for each port:
	for (size_t i = 0; i < ports.size(); i++)
	{
		setup_listening_socket(ports[i], server_blocks);
	}

	// Initialize poll structure with the listening sockets, later adds client_fds
	init_pfds(pfds, server_blocks);

	// Start poll and iterate through server blocks:
	// For listening sockets, accept() any POLLIN and add to the pollfds
	// For client sockets, handle the requests.
		// For cgi sockets, 
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

		int size_snapshot = pfds.size();
		int i = -1;
		while (!g_signal && ++i < size_snapshot)
		{
			if (pfds[i].revents & (POLLIN | POLLOUT))
			{
				std::map<int, sockaddr_in>::iterator it = server_blocks.find(pfds[i].fd);
				if (it != server_blocks.end()) // is one of the listeners
				{
					if (accept_client(pfds, it) == 1)
						reqs[pfds[i].fd] = Request(pfds[i].fd);
				}
				else if (reqs[pfds[i].fd].is_cgi) // is CGI request
				{
					char readbuff[BUFF_SZ];
					ssize_t bytes_read = 0;
					bytes_read = read(pfds[i].fd, readbuff, BUFF_SZ);
					if (bytes_read)
					{
						cgi.response.append(readbuff);
						// readbuff[bytes_read] = 0;
						// std::cout << "readbuff: " << readbuff << std::endl;
					}
					// std::cout << "CGI response: " << cgi.response << std::endl;
					else
					{
						close(pfds[i].fd);
					}
				}
				else if (pfds[i].revents & POLLIN) // is established client and has a request
				{
					char buf[BUFF_SZ];

					memset(buf, 0, BUFF_SZ);
					int nbytes = recv(pfds[i].fd, buf, BUFF_SZ, MSG_DONTWAIT);

					if (nbytes <= 0)  // error or finished reading the request
					{
						if (nbytes == 0)
							std::cout << "poll: socket " << pfds[i].fd << " hung up\n";
						else 
						{
							std::perror("recv");
							close(pfds[i].fd);
							// throw out the current fds[i]
							pfds[i] = pfds[size_snapshot - 1];
							size_snapshot--;
						}
					}
					else  // read the request
					{
						//if buffer has a chunk, continue reading (set POLLIN again)
						//if buffer has a full request parse it and handle it
						//if buffer has cgi request, handle it (after parsing)

						// if (pfds[i].fd == cgi.pipe[0]) // is cgi response
						//		cgi.response = strdup(buf);
						
						CgiHandler cgi(pfds[i].fd);
						if (cgi.cgi_flag)  //
						{
							cgi.cgi_flag = false;  //
							cgi.request = strdup(buf);
							cgi.client_fd = pfds[i].fd;
							handle_cgi(pfds, cgi, pipe_ends_for_cgi);
							// pfds[i].events = POLLOUT;
						}
						std::cout << "read "<< nbytes << " bytes: " << buf << std::endl;
					}
				}
				else if (pfds[i].revents & POLLOUT) // is established client and our response is ready
				{
					// send response
					//if (pfds[i].fd == cgi.client_fd) // && check if cgi response is ready with POLLHUP
					//	send(pfds[i].fd, cgi.response.c_str(), strlen(cgi.response.c_str()), MSG_DONTWAIT);
					//else
					//	send(pfds[i].fd, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!", 78, MSG_DONTWAIT);
					send(pfds[i].fd, cgi.response.c_str(), strlen(cgi.response.c_str()), MSG_DONTWAIT);
					close(pfds[i].fd);
					// throw out the current fds[i]
					pfds[i] = pfds[size_snapshot - 1];
					size_snapshot--;
				}
			}
		}
	}
	//cleanup
	for (std::vector<struct pollfd>::iterator it = pfds.begin(); it != pfds.end(); it++)
		close(it->fd);
}


// Close connections



