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

	std::string get_host() { return _host; };
	std::string get_request() { return _request; };
	std::string	get_request_body() { return _request_body; };
	std::string get_response() { return _response; };
	int			get_response_status() { return _response_status; };
	int			get_total_sent() { return _total_sent; };
	int			get_content_length() { return _content_length; };
	int			get_content_type_idx() { return _content_type_idx; };
	int			get_client_fd() { return _client_fd; };
	int			get_method() { return _method; };
	int			get_major_http_v() { return _major_http_v; };
	int			get_minor_http_v() { return _minor_http_v; };
	std::string get_request_location() { return _request_location; };
	int			get_cgi_status() { return _cgi_status; };
	std::string get_cgi_job_id() { return _cgi_job_id; };
	std::string get_cgi_output() { return _cgi_output; };
	std::vector<std::string> get_accepted_types() { return _accepted_types; };

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
	std::string _request_location;

	bool		_keep_alive;		// default: true

	int			_cgi_status;
	CgiHandler  _cgi;
	std::string _cgi_job_id;
	std::string _cgi_output;

};