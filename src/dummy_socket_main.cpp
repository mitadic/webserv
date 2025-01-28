#include "webserv.hpp"

// struct sockaddr_in {
// 	sa_family_t		sin_family;	/* address family: AF_INET */
// 	in_port_t		sin_port;	/* port in network byte order */
// 	struct in_addr	sin_addr;	/* internet address */
// };

// $ telnet localhost 9999

#define PORT 9999

int	main(void)
{
	// create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // int socket(int domain, int type, int protocol);
	if (sockfd < 0) {
		std::cerr << "Failed to create a socket. errno: " << errno << std::endl;
		return (1);
	}

	// specify address and port to assign to the socket
	// sockaddr_in	mysockaddr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(9999) };
	sockaddr_in	mysockaddr;
	mysockaddr.sin_family = AF_INET;
	mysockaddr.sin_addr.s_addr = INADDR_ANY;
	mysockaddr.sin_port = htons(9999);
	socklen_t	addrlen = sizeof(mysockaddr);

	// assign IP address and port to the socket
	if (bind(sockfd, (struct sockaddr*)&mysockaddr, addrlen) < 0) {  // int bind(int sockfd, const sockaddr *addr, socklen_t addrlen);
		std::cerr << "Failed to bind to port 9999. errno: " << errno << std::endl;
		return (1);
	}

	// start listening / mark a socket as passive, so that it can be used to accept connections (queue size <backlog>)
	if (listen(sockfd, 10) < 0) {	 // int listen(int sockfd, int backlog)
		std::cerr << "Failed to listen on socket " << sockfd << ". errno: " << errno << std::endl;
		return (1);
	}

	// extract an element from a queue of connections for a socket
	int connection = accept(sockfd, (struct sockaddr*)&mysockaddr, &addrlen);  // int accept(int sockfd, sockaddr *addr, socklen_t *addrlen);
	if (connection < 0) {
		std::cout << "Failed to grab connection. errno: " << errno << std::endl;
		return (1);
	}

	// Read from the connection
	char buffer[4096];
	while (1)
	{
		ssize_t bytesRead = read(connection, buffer, 100);
		if (bytesRead < 0) {
			std::cout << "Failed to grab connection. errno: " << errno << std::endl;
			return (1);
		}
		if (bytesRead == 0)
			break;
		buffer[bytesRead] = '\0';
		std::cout << "The message was: " << buffer;  // includes the \n

		// Send a message to the connection
		std::string response = "Good talking to you\n";
		send(connection, response.c_str(), response.size(), 0);
	}
	// Close the connections
	close(connection);
	close(sockfd);
}