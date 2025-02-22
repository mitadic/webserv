#include "CgiHandler.hpp"

#define GET 101
#define POST 102
#define DELETE 103

class Request {
public:
    Request() {}
    ~Request() {}

	std::string host;            // Host: example.com
	std::string mime_type;       // Content-Type: application/json (refers to own payload)
	std::string request;
    std::string response;
	int			total_sent;
	long long   content_length;  // Content-Length: 27
	short       method;          // GET POST DELETE
	int         client_fd;

	bool		is_cgi;
	CgiHandler  cgi;
	std::string cgi_job_id;
	std::string cgi_output;

private:

};