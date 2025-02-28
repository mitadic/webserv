#include "../incl/Request.hpp"

Request::Request() :
		_request(""),
		_response(""),
		_response_status(200),
		_total_sent(0),
		_method(UNINITIALIZED),
		_client_fd(UNINITIALIZED),
		_content_type_idx(UNINITIALIZED),
		_content_length(UNINITIALIZED),
		_flagged_as_chunked(false),
		_timed_out(false),
		_await_reconnection(false),
		_keep_alive(true),
		_cgi_status(NOT_CGI)
	{}

Request::~Request() {};

Request::Request(const Request& oth) : _cgi()
{
	(void)oth;
}


void Request::reset()
{
	_request.clear();
	_response.clear();
	_response_status = 200;
	_total_sent = 0;
	_method = UNINITIALIZED;
	_cgi_status = NOT_CGI;
	_cgi_output.clear();
}

void Request::reset_client()
{
	_request.clear();
	_response.clear();
	_response_status = 200;
	_method = UNINITIALIZED;
	_client_fd = -1;
	_total_sent = 0;
	_cgi_status = NOT_CGI;
	_cgi_output.clear();
}


int Request::parse()
{
	std::istringstream stream(_request);
	std::string line;
	RequestParser parser;

	spin_through_leading_crlf(stream, line);

	parser.parse_request_line(*this, stream, line);

	// Parse the headers
	while (!is_empty_crlf(line) && !stream.eof())
		parser.parse_header_line(*this, stream, line);

	// intermittent validation to prevent reading an endless body of content_type not defined or 'host' unspecified?

	// read body (if any) until another CRLF, then we're done
	while (std::getline(stream, line) && !is_empty_crlf(line))
		_request_body += line;

	// validate: here or in processing?

	return OK;
}