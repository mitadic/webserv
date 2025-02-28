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

class RequestParser;


class Request {

	friend class RequestParser;

public:
    Request();
    ~Request();
	Request(const Request&);

	void	reset();
	void	reset_client();
	int		parse();

	const std::string get_request() const;
	const std::string get_request_body() const;
	const std::string get_response() const;
	const std::string get_request_uri() const;
	const std::string get_cgi_job_id() const;
	const std::string get_cgi_output() const;
	const int get_response_status() const;
	const int get_total_sent() const;
	const int get_content_length() const;
	const int get_content_type_idx() const;
	const int get_client_fd() const;
	const int get_method() const;
	const int get_major_http_v() const;
	const int get_minor_http_v() const;
	const int get_cgi_status() const;
	const short get_port() const;
	const in_addr_t get_host() const;
	const std::vector<std::string> get_accepted_types() const;

	bool is_flagged_as_chunked();

	bool timed_out();
	bool should_await_reconnection();
	bool should_keep_alive();

	void set_response(std::string&);
	void append_to_response(std::string&);
	void set_response_status(int code);


private:
	std::string _request;
	std::string	_request_body;
	std::string _response;
	short		_port;
	in_addr_t	_host;
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