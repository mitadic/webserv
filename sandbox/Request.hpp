#include "CgiHandler.hpp"

#define GET 101
#define POST 102
#define DELETE 103

class Request {
public:
    Request(int fd) : client_fd(fd), is_cgi(false) {}
    ~Request() {}


	std::string host;            // Host: example.com
	std::string mime_type;       // Content-Type: application/json (refers to own payload)
	std::string request;
    std::string response;
	long long   content_length;  // Content-Length: 27
	short       method;          // GET POST DELETE
	int         client_fd;
	bool        is_cgi;
	CgiHandler  cgi;
private:
    Request() {}
};