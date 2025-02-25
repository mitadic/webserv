#pragma once

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
#include <fstream>

#include "Types.hpp"
#include "Request.hpp"
#include "CgiHandler.hpp"

#define MAX_SERVER_BLOCKS 50
#define MAX_CONNECTIONS 500
#define CONNECTION_TIMEOUT 5000
#define BUF_SZ 2
#define OK 0


extern volatile std::sig_atomic_t g_signal;  // declaration, 'extern' avoids multiple defs. Init in main.cpp


class ServerEngine {
public:
    ServerEngine();
    ~ServerEngine();

	void	run();
	bool	make_non_blocking(int &fd);
	void	setup_listening_socket(int port);
	void	init_listener_pfds();
	void	accept_client(int listener_fd, pfd_info meta);
	void	set_response(std::vector<pollfd>::iterator& pfds_it, int idx);
	void	set_basic_response(std::vector<pollfd>::iterator& pfds_it, int idx, std::string response);

	void	print_pfds();

	static void	signal_handler(int signal);

private:
	std::vector<int> ports;  // will be contained by vector<ServerBlock> in the future
	std::map<int, pfd_info> pfd_info_map;  // pfd.fd to all meta
	std::vector<struct pollfd> pfds;
	std::vector<Request> reqs;
};