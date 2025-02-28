#pragma once

#include <sstream>
#include <climits>
#include "CgiHandler.hpp"
#include "HttpHeaders.hpp"
#include "Exceptions.hpp"
#include "StatusCodes.hpp"
#include "ContentTypes.hpp"
#include "RequestUtils.hpp"
#include "RequestParser.hpp"

class RequestParser;  // forward declaration


class Request {

	friend class RequestParser;

public:
    Request();
    ~Request();
	Request(const Request&);

	void	reset();
	void	reset_client();
	int		parse();

	const std::string get_host() const { return _host; };
	const std::string get_request() const { return _request; };
	const std::string get_request_body() const { return _request_body; };
	const std::string get_response() const { return _response; };
	const std::string get_request_uri() const { return _request_uri; };
	const std::string get_cgi_job_id() const { return _cgi_job_id; };
	const std::string get_cgi_output() const { return _cgi_output; };
	const int get_response_status() const { return _response_status; };
	const int get_total_sent() const { return _total_sent; };
	const int get_content_length() const { return _content_length; };
	const int get_content_type_idx() const { return _content_type_idx; };
	const int get_client_fd() const { return _client_fd; };
	const int get_method() const { return _method; };
	const int get_major_http_v() const { return _major_http_v; };
	const int get_minor_http_v() const { return _minor_http_v; };
	const int get_cgi_status() const { return _cgi_status; };
	const std::vector<std::string> get_accepted_types() const { return _accepted_types; };

	bool		is_flagged_as_chunked() { return _flagged_as_chunked; };

	// execution relevant
	bool		timed_out() { return _timed_out; };
	bool		should_await_reconnection() { return _await_reconnection; };
	bool		should_keep_alive() { return _keep_alive; };

private:
	std::string _host;
	std::string _request;
	std::string	_request_body;
	std::string _response;
	int			_response_status;
	int			_total_sent;
	int			_content_length;  // refers to body
	bool		_flagged_as_chunked;
	int			_content_type_idx;  // content_types[n] || macros: TEXT_PLAIN, IMAGE_JPG
	int			_client_fd;
	bool		_timed_out;
	bool		_await_reconnection;
	std::vector<std::string> _accepted_types;

	int			_method;			// if GET POST DELETE
	int			_major_http_v;
	int			_minor_http_v;
	std::string _request_uri;

	bool		_keep_alive;		// default: true

	int			_cgi_status;
	CgiHandler  _cgi;
	std::string _cgi_job_id;
	std::string _cgi_output;

};