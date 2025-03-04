#include "Request.hpp"
#include "RequestParser.hpp"  // it's ok we're not including this in Request.hpp and it works due to fwd decl

Request::Request() :
	_request_str(""),
	_response(""),
	_response_status(CODE_200),
	_total_sent(0),
	_method(UNINITIALIZED),
	_client_fd(UNINITIALIZED),
	_content_type_idx(UNINITIALIZED),
	_content_length(UNINITIALIZED),
	_flagged_as_chunked(false),
	_timed_out(false),
	_await_reconnection(false),
	_keep_alive(true),
	_port(80),  // default for when unspecified
	_host(0x00000000),  // set to 0.0.0.0 bc a client may never request that?
	_cgi_status(NOT_CGI),
	_minor_http_v(UNINITIALIZED),
	_major_http_v(UNINITIALIZED),
	cgi()
{}

/* Parametrized constructor for when accepting client */
Request::Request(uint16_t port, in_addr_t host) :
	_request_str(""),
	_response(""),
	_response_status(CODE_200),
	_total_sent(0),
	_method(UNINITIALIZED),
	_client_fd(UNINITIALIZED),
	_content_type_idx(UNINITIALIZED),
	_content_length(UNINITIALIZED),
	_flagged_as_chunked(false),
	_timed_out(false),
	_await_reconnection(false),
	_keep_alive(true),
	_port(port),
	_host(host),
	_cgi_status(NOT_CGI),
	_minor_http_v(UNINITIALIZED),
	_major_http_v(UNINITIALIZED),
	cgi()
{}

Request::~Request() {};

Request::Request(const Request& oth) : cgi()
{
	_request_str = oth._request_str;
	_response = oth._response;
	_response_status = oth._response_status;
	_total_sent = oth._total_sent;
	_method = oth._method;
	_client_fd = oth._client_fd;
	_content_type_idx = oth._content_type_idx;
	_content_length = oth._content_length;
	_flagged_as_chunked = oth._flagged_as_chunked;
	_timed_out = oth._timed_out;
	_await_reconnection = oth._await_reconnection;
	_keep_alive = oth._keep_alive;
	_port = oth._port;
	_host = oth._host;
	_minor_http_v = oth._minor_http_v;
	_major_http_v = oth._major_http_v;
	_cgi_status = oth._cgi_status;
}


void Request::reset()
{
	_request_str.clear();
	_response.clear();
	_response_status = CODE_200;
	_total_sent = 0;
	_method = UNINITIALIZED;
	_cgi_status = NOT_CGI;
	_cgi_output.clear();
}

void Request::reset_client()
{
	reset();
	_client_fd = UNINITIALIZED;
}


const std::string Request::get_request_str() const { return _request_str; }
const std::string Request::get_request_body() const { return _request_body; }
const std::string Request::get_response() const { return _response; }
const std::string Request::get_request_uri() const { return _request_uri; }
const std::string Request::get_cgi_job_id() const { return _cgi_job_id; }
const std::string Request::get_cgi_output() const { return _cgi_output; }
const int Request::get_response_status() const { return _response_status; }
const int Request::get_total_sent() const { return _total_sent; }
const int Request::get_content_length() const { return _content_length; }
const int Request::get_content_type_idx() const { return _content_type_idx; }
const char *Request::get_content_type() const { return content_types[_content_type_idx]; }
const int Request::get_client_fd() const { return _client_fd; }
const int Request::get_method() const { return _method; }
const int Request::get_major_http_v() const { return _major_http_v; }
const int Request::get_minor_http_v() const { return _minor_http_v; }
const int Request::get_cgi_status() const { return _cgi_status; }

/* Get the port_no specified in the request; it has been validated to fit the legal range for ports */
const uint16_t Request::get_port() const { return _port; }

/* Get the host specified in the request; it has been confirmed to fit between 0.0.0.0 and 255.255.255.254 */
const in_addr_t Request::get_host() const { return _host; }

/* Get the Accept specified types, sorted by priority */
const std::vector<std::string> Request::get_accepted_types() const { return _accepted_types; }

/* See if the request has specified Content-Type to be "chunked" in the headers */
bool Request::is_flagged_as_chunked() { return _flagged_as_chunked; }

/* If a request has timed out, then ServerEngine can handle it accordingly */
bool Request::timed_out() { return _timed_out; }

/* If a request should ditch the client and wait for a new connection to be established */
bool Request::should_await_reconnection() { return _await_reconnection; }

/* If "Connection: keep alive" (default), as opposed to "Connection: close" */
bool Request::should_keep_alive() { return _keep_alive; }


/** @brief Append string to _request_str
 * @param s The string to append
 */
void Request::append_to_request_str(const std::string& s)
{
	_request_str += s;
}

/* Clear any existing _response before setting it to be the argument string */
void Request::set_response(const std::string& s)
{
	_response.clear();
	_response = s;
}

/* Append string to the _response */
void Request::append_to_response(const std::string& s) { _response += s; }

/* Overwrite the default 200 */
void Request::set_response_status(const int& code) { _response_status = code; }

/* Set _total_sent */
void Request::set_total_sent(const int& num) { _total_sent = num; }

/* Set _cgi_status */
void Request::set_cgi_status(const int& status) { _cgi_status = status; }

/* Append string to _cgi_output */
void Request::append_to_cgi_output(const std::string& s) { _cgi_output += s; }

/* Increment _total_sent value by num */
void Request::increment_total_sent_by(const int& num) { _total_sent += num; }

/* Set _timed_out to 'true' */
void Request::flag_the_timeout() { _timed_out = true; }


/*
Verify existence of: (1) host, (2) content_length if method is POST.
Go through map of q:accepted_type, which is always sorted, and push_back just the types in reverse.
*/
void Request::validate_self()
{
	if (_host == 0x00000000)
		throw RequestException(CODE_400);
	if (_method == POST && _content_length == UNINITIALIZED)
		throw RequestException(CODE_411);
	if (_major_http_v > 1 || _major_http_v < 0
			|| (_major_http_v == 1 && _minor_http_v > 1)
			|| (_major_http_v == 0 && _minor_http_v < 9))
		throw RequestException(CODE_505);

	for (std::map<float, std::string>::reverse_iterator it = _accepted_types_m.rbegin(); it != _accepted_types_m.rend(); it++)
		_accepted_types.push_back(it->second);
}

<<<<<<< HEAD
int Request::parse()
=======
/* Includes request validation before parsing the body */
void Request::parse()
>>>>>>> 2ea2ec28c4b8828f90bfda76b92ae454a49fad32
{
	std::istringstream stream(_request_str);
	std::string line;
	RequestParser parser;

	spin_through_leading_crlf(stream, line);

	parser.parse_request_line(*this, stream, line);
	parser.parse_headers(*this, stream, line);
	validate_self();
	parser.parse_body(*this, stream, line);
}