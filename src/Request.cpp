#include "Request.hpp"
#include "RequestParser.hpp" // it's ok we're not including this in Request.hpp and it works due to fwd decl

Request::Request() :
	cgi(NULL),
	_port(80),  // default for when unspecified
	_host(0x00000000),  // set to 0.0.0.0 bc a client may never request that?
	_response_status(status_code_values[CODE_200]),
	_total_sent(0),
	_content_length(UNINITIALIZED),
	_flagged_as_chunked(false),
	_done_reading_headers(false),
	_done_reading_chunked_body(false),
	_should_close_early(false),
	_content_type_idx(UNINITIALIZED),
	_client_fd(UNINITIALIZED),
	_keep_alive(true),
	_await_reconnection(false),
	_method(UNINITIALIZED),
	_major_http_v(UNINITIALIZED),
	_minor_http_v(UNINITIALIZED),
	_cgi_status(NOT_CGI),
	_chunk_size(UNINITIALIZED),
	_nread_in_chunk_size(UNINITIALIZED)
{}

/* Parametrized constructor for when accepting client */
Request::Request(in_addr_t host, uint16_t port, int client_fd) :
	cgi(NULL),
	_port(port),
	_host(host),
	_response_status(status_code_values[CODE_200]),
	_total_sent(0),
	_content_length(UNINITIALIZED),
	_flagged_as_chunked(false),
	_done_reading_headers(false),
	_done_reading_chunked_body(false),
	_should_close_early(false),
	_content_type_idx(UNINITIALIZED),
	_client_fd(client_fd),
	_keep_alive(true),
	_await_reconnection(false),
	_method(UNINITIALIZED),
	_major_http_v(UNINITIALIZED),
	_minor_http_v(UNINITIALIZED),
	_cgi_status(NOT_CGI),
	_chunk_size(UNINITIALIZED),
	_nread_in_chunk_size(UNINITIALIZED)
{}

Request::~Request() {
	if (cgi) {
		delete cgi;
		cgi = NULL;
	}
}

Request::Request(const Request& oth) : cgi(NULL)
{
	if (oth.cgi)
		this->cgi = new CgiHandler(*(oth.cgi));  // uses CgiHandler copy constructor
	_request_str = oth._request_str;
	_response = oth._response;
	_response_status = oth._response_status;
	_total_sent = oth._total_sent;
	_method = oth._method;
	_client_fd = oth._client_fd;
	_content_type_idx = oth._content_type_idx;
	_content_length = oth._content_length;
	_flagged_as_chunked = oth._flagged_as_chunked;
	_done_reading_headers = oth._done_reading_headers;
	_done_reading_chunked_body = oth._done_reading_chunked_body;
	_should_close_early = oth._should_close_early;
	_await_reconnection = oth._await_reconnection;
	_keep_alive = oth._keep_alive;
	_port = oth._port;
	_host = oth._host;
	_minor_http_v = oth._minor_http_v;
	_major_http_v = oth._major_http_v;
	_cgi_status = oth._cgi_status;
	_chunk_size = oth._chunk_size;
	_nread_in_chunk_size = oth._nread_in_chunk_size;
}

const std::string& Request::get_request_str() const { return _request_str; }
const std::string Request::get_request_body_as_str() const {
	std::string body_as_str;
	for (std::vector<unsigned char>::const_iterator it = _request_body.begin(); it != _request_body.end(); it++)
		body_as_str += *it;
	return body_as_str;
}
const std::vector<unsigned char> &Request::get_request_body_raw() const { return _request_body; }
const std::string& Request::get_response() const { return _response; }
const std::string& Request::get_request_uri() const { return _request_uri; }
const std::string& Request::get_request_query_string() const { return _query_string; }
const std::string& Request::get_cgi_job_id() const { return _cgi_job_id; }
const std::string& Request::get_cgi_output() const { return _cgi_output; }
const int& Request::get_response_status() const { return _response_status; }
const int& Request::get_total_sent() const { return _total_sent; }
const int64_t& Request::get_content_length() const { return _content_length; }
const int& Request::get_content_type_idx() const { return _content_type_idx; }
const std::vector<std::string>& Request::get_content_type_params() const { return _content_type_params; }
const char *Request::get_content_type() const {
	if (_content_type_idx == UNINITIALIZED)
		return "";
	return content_types[_content_type_idx];
}
const int& Request::get_client_fd() const { return _client_fd; }
const int& Request::get_method() const { return _method; }
const int& Request::get_major_http_v() const { return _major_http_v; }
const int& Request::get_minor_http_v() const { return _minor_http_v; }
const int& Request::get_cgi_status() const { return _cgi_status; }

/* Get the port_no specified in the request; it has been validated to fit the legal range for ports */
const uint16_t& Request::get_port() const { return _port; }

/* Get the host specified in the request; it has been confirmed to fit between 0.0.0.0 and 255.255.255.254 */
const in_addr_t& Request::get_host() const { return _host; }

/* Get the Accept specified types, sorted by priority */
const std::vector<std::string>& Request::get_accepted_types() const { return _accepted_types; }

/* See if the request has specified Content-Type to be "chunked" in the headers */
bool Request::is_flagged_as_chunked() { return _flagged_as_chunked; }

/* See if chunked body has been read to the point of no new chunk_size specified */
bool Request::done_reading_chunked_body() { return _done_reading_chunked_body; }

/* See if finished reading the headers */
bool Request::done_reading_headers() { return _done_reading_headers; }

/* See if anything has indicated to close connection early */
bool Request::should_close_early() { return _should_close_early; }

/* Flip the attribute to indicate that we're now reading body */
void Request::switch_to_reading_body() { _done_reading_headers = true; }

/* Flip the attribute to indicate that we're done reading the chunked body */
void Request::flag_that_done_reading_chunked_body() { _done_reading_chunked_body = true; }

/* Flip the attribute to indicate that we should close the connection early */
void Request::flag_that_we_should_close_early() { _should_close_early = true; }

/* If a request should ditch the client and wait for a new connection to be established */
bool Request::should_await_reconnection() { return _await_reconnection; }

/* If "Connection: keep-alive" (default), as opposed to "Connection: close" */
bool Request::should_keep_alive() { return _keep_alive; }

/** @brief Append string to _request_str
 * @param s The string to append
 */
void Request::append_to_request_str(const std::string &s)
{
	_request_str += s;
}

/* Clear any existing _response before setting it to be the argument string */
void Request::set_response(const std::string &s)
{
	_response.clear();
	_response = s;
}

void Request::set_request_uri(const std::string &s)
{
	_request_uri = s;
}

/* Append string to the _response */
void Request::append_to_response(const std::string &s) { _response += s; }

/* Append unsigned char to _request_body */
void Request::append_byte_to_body(const unsigned char &c) { _request_body.push_back(c); }

/* Overwrite the default 200 */
void Request::set_response_status(const int &code) { _response_status = code; }

/* Set _total_sent */
void Request::set_total_sent(const int &num) { _total_sent = num; }

/* Set _cgi_status */
void Request::set_cgi_status(const int &status) { _cgi_status = status; }

/* Append string to _cgi_output */
void Request::append_to_cgi_output(const std::string &s) { _cgi_output += s; }

/* Increment _total_sent value by num */
void Request::increment_total_sent_by(const int &num) { _total_sent += num; }

const std::string Request::get_chunk_size_hex_str() const { return _chunk_size_hex_str; }
const int& Request::get_chunk_size() const { return _chunk_size; }
const int& Request::get_nread_of_chunk_size() const { return _nread_in_chunk_size; }
void Request::set_chunk_size_hex_str(const std::string s) { _chunk_size_hex_str = s; }
void Request::set_chunk_size(const int& n) { _chunk_size = n; }
void Request::set_nread_of_chunk_size(const int& n ) { _nread_in_chunk_size = n; }

const std::map<std::string, std::string> &Request::get_cookies() const
{
	return _cookies;
}

void Request::set_cookies(const std::string &cookie)
{
	std::istringstream stream(cookie);
	std::string single_cookie_key;
	std::string single_cookie_value;
	while (std::getline(stream, single_cookie_key, '='))
	{
		std::getline(stream, single_cookie_value, ';');
		_cookies[single_cookie_key] = single_cookie_value;
	}
}


/*
Verify existence of: (1) host, (2) content_length if method is POST.
Go through map of q:accepted_type, which is always sorted, and push_back just the types in reverse.
*/
void Request::validate_self()
{
	if (_host == 0x00000000)
		throw RequestException(CODE_400);
	if (_method == POST && _flagged_as_chunked == false && (_content_length == 0 || _content_length == UNINITIALIZED))
		throw RequestException(CODE_411);
	if (_major_http_v > 1 || _major_http_v < 0 || (_major_http_v == 1 && _minor_http_v > 1) || (_major_http_v == 0 && _minor_http_v < 9))
		throw RequestException(CODE_505);

	for (std::multimap<float, std::string>::reverse_iterator it = _accepted_types_m.rbegin(); it != _accepted_types_m.rend(); it++)
		_accepted_types.push_back(it->second);
}

/* Parses all headers and validates the Request before the body is recv()-ed */
void Request::parse()
{
	std::istringstream stream(_request_str);
	std::string line;
	RequestParser parser;

	spin_through_leading_crlf(stream, line);

	Log::log("Request line raw pre-parsing: " + line, DEBUG);
	parser.parse_request_line(*this, line);
	parser.parse_headers(*this, stream, line);
	validate_self();
}
