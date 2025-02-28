#pragma once

#include "CgiHandler.hpp"
#include "Utils.hpp"
#include <stdexcept>


class Request {
public:
    Request() :
		cgi_status(NOT_CGI),
		request(""),
		response(""),
		total_sent(0),
		client_fd(-1)
	{}
    ~Request() {}

	void reset()
	{
		request.clear();
		response.clear();
		client_fd = -1;
		total_sent = 0;
		cgi_status = NOT_CGI;
		cgi_output.clear();
	}

	std::string host;            // Host: example.com
	std::string mime_type;       // Content-Type: application/json (refers to own payload)
	std::string request;
    std::string response;
	std::string uri;             // /index.html
	int			total_sent;
	long long   content_length;  // Content-Length: 27
	short       method;          // GET POST DELETE
	int         client_fd;

	int			cgi_status;
	CgiHandler  cgi;
	std::string cgi_job_id;
	std::string cgi_output;

private:

};