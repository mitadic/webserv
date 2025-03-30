#pragma once

#include <map>
#include <vector>
#include <cerrno> 		// For errno
#include <cstdlib>
#include <cstdio>		// perror()
#include <ctime>
#include <poll.h>
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>		// close()

enum e_retval {
	OK = 0,
	NOT_OK
};

enum e_pfd_type
{
	CLIENT_CONNECTION_SOCKET = 100,  // the only one that we directly map to a request
	LISTENER_SOCKET,
	CGI_PIPE_IN,
	CGI_PIPE_OUT  // indirectly mapped to a request through Cgi, which itself is part of a Request
};

/* Request-related info */
enum e_cgi_status
{
	NOT_CGI = 200,
	EXECUTE,
	CGI_DONE
};

/** Initialized and pushed_back for each client_fd
 * @param reqs_idx needed only by CONNECTION, PIPE
 * @param sockaddr needed only by LISTENER + socket_addr.sin_port has the [port]
 * @param host mapping pretend IP [host] for req and processing
 * @param port representation from server_block in READABLE endianness
 * @param max_client_body
 * @param last_active needed only by CONNECTION; connection established | (done writing? | began reading?)
 * @param cgi_pid needed only by CGI_PIPE_IN and CGI_PIPE_OUT to perform waitpid()
 * */
struct pfd_info {
	int type;

	int reqs_idx;			// needed only by CONNECTION, PIPE
	sockaddr_in sockaddr;	// needed only by LISTENER + socket_addr.sin_port has the [port]
	in_addr_t host;			// mapping pretend IP [host] for req and processing
	uint16_t port;			// representation from server_block in READABLE endianness
	uint32_t max_client_body;
	time_t last_active;		// needed only by CONNECTION; connection established | (done writing? | began reading?)
	pid_t cgi_pid;			// needed only by CGI_PIPE_IN and CGI_PIPE_OUT
};