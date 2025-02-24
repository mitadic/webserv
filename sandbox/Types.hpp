#pragma once

#define GET 101
#define POST 102
#define DELETE 103

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
	sockaddr_in sockaddr;	// needed only by LISTENER
};