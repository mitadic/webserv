#include "../../incl/webserv.hpp"

#define PORT 9999
#define MAX_EVENTS 10


void set_socket_nonblocking(int socket) {
	int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

int	create_non_blocking_server_socket_bind_listen(void)
{
	int	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Allow socket reuse
	int	opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// Specify address and port to be assigned to the socket
	struct sockaddr_in mysockaddr;
	mysockaddr.sin_family = AF_INET;
	mysockaddr.sin_addr.s_addr = INADDR_ANY;
	mysockaddr.sin_port = htons(PORT);

	// assign IP address and port to the socket
	if (bind(server_fd, (struct sockaddr*)&mysockaddr, sizeof(mysockaddr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

	// start listening / mark a socket as passive, so that it can be used to accept connections (queue size <backlog>)
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Set the listening socket to non-blocking mode
    set_socket_nonblocking(server_fd);

    return server_fd;
}

int main(int argc, char* argv[])
{
	// Register signal handlers

	// Parse config file
		// Determine config file path from command line or use default
		// Parse config file 
		// Store config settings in data structures

	// Set up server sockets
		// For each server block:
			// Create socket
			// Set socket to be non-blocking
			// Bind to specified host/port  
			// Start listening
			int	server_fd = create_non_blocking_server_socket_bind_listen();

	// Start event loop
		// Create epoll instance (via this fd we refer to the central API DS, it's not used for I/O)
		int epoll_fd = epoll_create1(0);
		if (epoll_fd == -1) {
			perror("epoll_create1 failed");
			close(server_fd);
			exit(EXIT_FAILURE);
		}

		// Add server socket to epoll interest list
		struct epoll_event ev;
		ev.events = EPOLLIN; // Wait for incoming connections
		ev.data.fd = server_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev); // Register the listening socket with epoll

		struct epoll_event events[MAX_EVENTS];

		// Loop:
		while (true)
		{
			// Call epoll_wait
			int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
			if (num_events < 0) {
				perror("epoll_wait failed");
				break;
			}

			for (int i = 0; i < num_events; i++)
			{
				// Handle new connections 
				if (events[i].data.fd == server_fd)
				{
					struct sockaddr_in client_addr;
					socklen_t client_len = sizeof(client_addr);
					int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
					if (client_fd < 0) {
						perror("accept failed");
						continue;
					}

					set_socket_nonblocking(client_fd); // Make client socket non-blocking

					struct epoll_event client_event;
					client_event.events = EPOLLIN | EPOLLET; // Edge-triggered mode
					client_event.data.fd = client_fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);

					std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << std::endl;
				}
				else
				{
					// Handle client requests (basic read & respond)
					char buffer[1024] = {0};
					int client_fd = events[i].data.fd;
					int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

					if (bytes_read <= 0) {
						// Connection closed or error
						close(client_fd);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
						std::cout << "Client disconnected" << std::endl;
					}
					// Handle client responses
					else
					{
						// Simple HTTP response
						std::cout << "Received: " << buffer;
						const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World";
						write(client_fd, response, strlen(response));
					}
				}
			}
		}	
		// Break loop on shutdown signal

	// Clean up
		// Close all sockets
		close(server_fd);
    	close(epoll_fd);
		// Free memory
}