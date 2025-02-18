/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbencze <pbencze@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 11:04:22 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/18 14:31:51 by pbencze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>		// close()
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <poll.h>
#include <algorithm>
#include <csignal> // For signal handling
#include <cerrno> // For errno

#define MAX_SERVER_BLOCKS 50
#define MAX_CONNECTIONS 500
#define CONNECTION_TIMEOUT 5000
#define CHUNK_SZ 256

volatile std::sig_atomic_t g_signal = 0;

void signal_handler(int signal)
{
	g_signal = signal;
}

// Create the listening socket for all ports using IPv4 and TCP (Socket Stream) and store them in a map
// Create the address struct for all ports, bind and start listening (passive sockets)
void setup_listening_socket(int port, std::map<int, sockaddr_in>& map)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
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

int main()
{
  std::signal(SIGINT, signal_handler); // handles Ctrl+C
	std::vector<int> ports = { 9992, 9993, 9991 };
	std::map<int, sockaddr_in > server_blocks;

	// Setup listening sockets for each port:
	for (int i = 0; i < ports.size(); i++)
	{
		setup_listening_socket(ports[i], server_blocks);
	}

	// Initialize poll structure with the listening sockets, later adds client_fds
	std::vector<struct pollfd> pfds;
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

	// Start poll and iterate through server blocks:
	// For listening sockets, accept() any POLLIN and add to the pollfds. // TODO: NONBLOCK
	// For client sockets, handle the requests. // TODO: Needs to be NONBLOCK as well?
	while (!g_signal)
	{
		int timeout = CONNECTION_TIMEOUT;
		int events_count = poll(&pfds[0], pfds.size(), timeout);
		if (events_count == -1)
		{
			//if SIGINT (Ctrl+C) is received, exit gracefully
			if (errno == EINTR) {
				std::cout << "Signal received. Exiting..." << std::endl;
				exit(EXIT_SUCCESS);
			}
			std::cout << "Poll failed. Errn: " << errno << std::endl;
			continue;
		}

		int size_snapshot = pfds.size();
		int i = -1;
		while (!g_signal && ++i < size_snapshot)
		{
			if (pfds[i].revents & POLLIN)
			{
				std::map<int, sockaddr_in>::iterator it = server_blocks.find(pfds[i].fd);
				if (it != server_blocks.end())
				// is one of the listeners
				{
					int addrlen = sizeof(it->second);
					int client = accept(it->first, (struct sockaddr*)&it->second, (socklen_t*)&addrlen);
					if (client == -1)
					{
						std::cout << "Failed to grab connection. Errn: " << errno << std::endl;
						continue;
					}
					struct pollfd fd;
					fd.fd = client;
					// std::cout << fd.fd << std::endl;
					fd.events = POLLIN;
					if (pfds.size() >= MAX_CONNECTIONS)
					{
						std::cout << "Max connections reached." << std::endl;
						continue;
					}
					pfds.push_back(fd);
				}
				else
				// is established client
				{
					char buf[CHUNK_SZ];

					int nbytes = recv(pfds[i].fd, buf, CHUNK_SZ, 0);
					if (nbytes <= 0)
					{
						if (nbytes == 0)
							std::cout << "poll: socket " << pfds[i].fd << " hung up\n";
						else
							perror("recv");
						close(pfds[i].fd);
						// throw out the current fds[i]
						pfds[i] = pfds[size_snapshot - 1];
						size_snapshot--;
					}
					else
					{
						std::cout << buf;
					}
				}
			}
		}
	}
	//handle graceful exit on SIGINT
}


// Close connections
