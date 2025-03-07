#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>

#include <cstdlib>
#include <cstdio>
#include <cerrno> // For errno
#include <cstring>

#include <unistd.h>		// close()
#include <fcntl.h> // For fcntl
#include <poll.h>
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in

#include "Types.hpp"
#include "ServerBlock.hpp"
#include "SignalHandling.hpp"
#include "Request.hpp"
#include "CgiHandler.hpp"
#include "Exceptions.hpp"
#include "Config.hpp"
#include "ErrorPageGenerator.hpp"

#define MAX_SERVER_BLOCKS 50
#define MAX_CONNECTIONS 500
#define CONNECTION_TIMEOUT 50000
#define BUF_SZ 256


class ServerEngine {
public:
	ServerEngine(std::vector<ServerBlock>&);
	~ServerEngine();

	void	run();
	bool	make_non_blocking(int &fd);
	void	setup_listening_socket(const ServerBlock& sb);
	void	init_listener_pfds();
	void	accept_client(int listener_fd, pfd_info meta);
	void	forget_client(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);
	void	initiate_error_response(std::vector<pollfd>::iterator&, int idx, int code);
	void	initialize_new_request_if_no_active_one(std::map<int, pfd_info>::iterator&);
	void	liberate_client_for_next_request(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);
	void	print_pfds();
	void	update_client_activity_timestamp(std::map<int, pfd_info>::iterator&);

	void	read_from_cgi_pipe(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);
	void	read_from_client_fd(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);
	void	write_to_client(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);
	void	process_eof_on_pipe(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);
	void	process_unorderly_hangup(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);
	void	process_connection_timeout(std::vector<pollfd>::iterator&, std::map<int, pfd_info>::iterator&);

	void	process_request(std::vector<pollfd>::iterator&, Request&);

	bool	is_client_and_timed_out(const pfd_info& pfd_meta);

	static void	signal_handler(int signal);

	std::vector<ServerBlock> server_blocks;
	std::map<int, pfd_info> pfd_info_map;  // pfd.fd to all meta
	std::vector<struct pollfd> pfds;
	std::vector<Request> reqs;

	bool pfds_vector_modified;

private:
	ServerEngine();
};