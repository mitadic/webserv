/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 11:04:22 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/17 16:53:46 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <poll.h>
#include <algorithm>

#define MAX_CONNECTIONS 500
#define CONNECTION_TIMEOUT 5000


// Create the listening socket for all ports using IPv4 and TCP (Socket Stream) and store them in a map
// Create the address struct for all ports, bind and start listening (passive sockets)
void setup_listening_sockets(int port, std::map<int, sockaddr_in>& map)
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
		std::cout << "Failed to bind to port " << port << ". Errno:" << errno << std::endl;
		exit(EXIT_FAILURE);
	}
	if (listen(sockfd, 10) == -1)
	{
		std::cout << "Failed to listen on socket. Errno: " << errno << std::endl;
		exit(EXIT_FAILURE);
	}

	map[sockfd] = socket_addr;
}


int main()
{
	std::vector<int> ports = {9992, 9993, 9991};
	std::map<int, sockaddr_in > connections;

	// Setup listening sockets for each port:
	for (int i = 0; i < ports.size(); ++i)
	{
		sockaddr_in socket_addr;
		setup_listening_sockets(ports[i], connections);
	}

	// Initialize poll structure with the listening sockets
	std::vector<struct pollfd> fds;
	for (std::map<int, sockaddr_in>::iterator it = connections.begin(); it != connections.end(); ++it)
	{
		struct pollfd fd;
		fd.fd = it->first;
		fd.events = POLLIN;
		if (fds.size() >= MAX_CONNECTIONS)
		{
			std::cout << "Max connections reached." << std::endl;
			exit(EXIT_FAILURE);
		}
		fds.push_back(fd);
	}

	// Start poll and iterate through existing connections:
	// For listening sockets, accept() any new connections and add to the pollfds. // TODO: NONBLOCK
	// For client sockets, handle the requests. // TODO: Needs to be NONBLOCK as well?
	for(;;)
	{
		int timeout = CONNECTION_TIMEOUT;
		int poll_count = poll(&fds[0], fds.size(), timeout);
		if (poll_count == -1)
		{
			std::cout << "Poll failed. Errn: " << errno << std::endl;
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				std::map<int, sockaddr_in>::iterator it = connections.find(fds[i].fd);
				if (it != connections.end())
				{
					int addrlen = sizeof(it->second);
					int client = accept(it->first, (struct sockaddr*)&it->second, (socklen_t*)&addrlen);
					if (client == -1)
						std::cout << "Failed to grab connection. Errn: " << errno << std::endl;
					else
					{
						struct pollfd fd;
						fd.fd = client;
						std::cout << fd.fd << std::endl;
						fd.events = POLLIN;
						if (fds.size() >= MAX_CONNECTIONS)
						{
							std::cout << "Max connections reached." << std::endl;
							exit(EXIT_FAILURE);
						}
						fds.push_back(fd);
					}
				}
				else
					std::cout << "Request will be handled" << std::endl; // TODO
			}
		}
	}
}


// Close connections