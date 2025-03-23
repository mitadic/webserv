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

#define CLIENT_CONNECTION_SOCKET 100  // the only one that we directly map to a request
#define LISTENER_SOCKET 101
#define CGI_PIPE 102  // indirectly mapped to a request through Cgi, which itself is part of a Request

/* Request-related info */
enum e_cgi_status
{
	NOT_CGI = 200,
	EXECUTE,
	READ_PIPE,
	FORMULATE_RESPONSE,
	AWAIT_CLIENT_RECONNECT  // won't be needing your services
};

/** Initialized and pushed_back for each client_fd
 * @param reqs_idx updated for each current request
 * */
struct pfd_info {
	int type;

	int reqs_idx;			// needed only by CONNECTION, PIPE
	sockaddr_in sockaddr;	// needed only by LISTENER + socket_addr.sin_port has the [port]
	in_addr_t host;			// mapping pretend IP [host] for req and processing
	uint16_t port;			// representation from server_block in READABLE endianness
	uint32_t max_client_body;
	time_t last_active;		// needed only by CONNECTION; connection established | (done writing? | began reading?)
	bool had_at_least_one_req_processed;  // needed only by CONNECTION, to differentiate between 408 and silent close
};