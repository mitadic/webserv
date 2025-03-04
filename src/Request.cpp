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


const std::string Request::get_request() const { return _request; };
const std::string Request::get_request_body() const { return _request_body; };
const std::string Request::get_response() const { return _response; };
const std::string Request::get_request_uri() const { return _request_uri; };
const std::string Request::get_cgi_job_id() const { return _cgi_job_id; };
const std::string Request::get_cgi_output() const { return _cgi_output; };
const int Request::get_response_status() const { return _response_status; };
const int Request::get_total_sent() const { return _total_sent; };
const int Request::get_content_length() const { return _content_length; };
const int Request::get_content_type_idx() const { return _content_type_idx; }
const int Request::get_client_fd() const { return _client_fd; }
const int Request::get_method() const { return _method; }
const int Request::get_major_http_v() const { return _major_http_v; }
const int Request::get_minor_http_v() const { return _minor_http_v; }
const int Request::get_cgi_status() const { return _cgi_status; }

/* Get the port specified in the request */
const short Request::get_port() const { return _port; }

/* Get the host specified in the request */
const in_addr_t Request::get_host() const { return _host; }


const std::vector<std::string> Request::get_accepted_types() const { return _accepted_types; }


bool Request::is_flagged_as_chunked() { return _flagged_as_chunked; }


bool Request::timed_out() { return _timed_out; }


bool Request::should_await_reconnection() { return _await_reconnection; }


bool Request::should_keep_alive() { return _keep_alive; }


/* Clear any existing _response before setting it to be the argument string */
void Request::set_response(std::string& s)
{
	_response.clear();
	_response = s;
}

/* Append string to the _response */
void Request::append_to_response(std::string& s)
{
	_response += s;
}

/* Overwrite the default 200 */
void Request::set_response_status(int code)
{
	_response_status = code;
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