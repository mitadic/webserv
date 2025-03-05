#pragma once

#include <map>
#include <vector>
#include <cerrno> 		// For errno
#include <cstdlib>		
#include <cstdio>		// perror()
#include <poll.h>
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>		// close()

enum e_retval {
	OK = 0,
	NOT_OK
};

#define CLIENT_CONNECTION_SOCKET 100  // the only one that we directly map to a request
#define LISTENER_SOCKET 101
#define CGI_PIPE 102  // indirectly mapped to a request through Cgi, which itself is part of a Request

#define NOT_CGI 200
#define EXECUTE 201
#define READ_PIPE 202
#define FORMULATE_RESPONSE 203
#define AWAIT_CLIENT_RECONNECT 204

struct pfd_info {
	int type;

	int reqs_idx;			// needed only by CONNECTION, PIPE
	sockaddr_in sockaddr;	// needed only by LISTENER + socket_addr.sin_port has the [port]
	in_addr_t host;			// mapping pretend IP [host] for req and processing
	uint16_t port;			// representation from server_block in whatever endianness
};