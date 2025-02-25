#include "Request.hpp"

Request::Request() :
		request(""),
		response(""),
		total_sent(0),
		client_fd(-1),
		timed_out(false),
		await_reconnection(false),
		cgi_status(NOT_CGI)
	{}

Request::~Request() {};

Request::Request(const Request& oth) : cgi()
{
    (void)oth;
}


void Request::reset()
{
    request.clear();
    response.clear();
    total_sent = 0;
    cgi_status = NOT_CGI;
    cgi_output.clear();
}

void Request::reset_client()
{
    request.clear();
    response.clear();
    client_fd = -1;
    total_sent = 0;
    cgi_status = NOT_CGI;
    cgi_output.clear();
}

void Request::parse()
{
    ;
}