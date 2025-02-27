#pragma once

#include <sstream>
#include <climits>
#include "CgiHandler.hpp"
#include "HttpHeaders.hpp"
#include "Exceptions.hpp"

#define SP " "
#define LWS_CHARS " \t"

#define FIVEHUNDRED "HTTP/1.1 500 Internal Server Error"
#define FIVEHUNDREDNUM 500


class Request {
public:
    Request();
    ~Request();
	Request(const Request&);

	void	reset();
	void	reset_client();

	int		parse();
	int		parse_request_line(std::string& line);
	void	parse_header_line(std::istringstream& stream, std::string& line);

	std::string host;            // Host: example.com
	std::string mime_type;       // Content-Type: application/json (refers to own payload)
	std::string request;
    std::string response;
	int			response_status;
	int			total_sent;
	long long   content_length;  // Content-Length: 27
	int         client_fd;
	bool		timed_out;
	bool		await_reconnection;

	char		method;          // GET POST DELETE
	int			major_http_v;
	int			minor_http_v;
	std::string request_location;

	bool		keep_alive;		// default: true

	int			cgi_status;
	CgiHandler  cgi;
	std::string cgi_job_id;
	std::string cgi_output;

private:

};