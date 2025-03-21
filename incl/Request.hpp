#pragma once

#include <sstream>
#include <map>
#include <climits>
#include "CgiHandler.hpp"
#include "HttpHeaders.hpp"
#include "Exceptions.hpp"
#include "StatusCodes.hpp"
#include "ContentTypes.hpp"
#include "RequestUtils.hpp"
#include "RequestProcessor.hpp"


class RequestParser;


class Request {

	friend class RequestParser;

public:
	Request(in_addr_t, uint16_t);
    ~Request();
	Request(const Request&);

	void	reset();
	void	reset_client();
	void	parse();
	void	validate_self();

	const std::string& get_request_str() const;
	const std::string get_request_body_as_str() const;
	const std::vector<unsigned char>& get_request_body_raw() const;
	const std::string& get_response() const;
	const std::string& get_request_uri() const;
	const std::string& get_cgi_job_id() const;
	const std::string& get_cgi_output() const;
	const int& get_response_status() const;
	const int& get_total_sent() const;
	const int& get_content_length() const;
	const int& get_content_type_idx() const;
	const char *get_content_type() const;
	const std::vector<std::string>& get_content_type_params() const;

	const int& get_client_fd() const;
	const int& get_method() const;
	const int& get_major_http_v() const;
	const int& get_minor_http_v() const;
	const int& get_cgi_status() const;
	const uint16_t& get_port() const;
	const in_addr_t& get_host() const;
	const std::vector<std::string>& get_accepted_types() const;

	bool is_flagged_as_chunked();
	bool done_reading_headers();
	void switch_to_reading_body();

	bool timed_out();
	bool should_await_reconnection();
	bool should_keep_alive();
	bool should_close_early();

	// void set_port(const uint16_t&);
	// void set_host(const in_addr_t&);
	void append_to_request_str(const std::string& s);
	void set_response(const std::string& s);
	void append_to_response(const std::string& s);
	void append_byte_to_body(const unsigned char& c);
	void set_response_status(const int& code);
	void set_total_sent(const int& num);
	void set_cgi_status(const int& status);
	void append_to_cgi_output(const std::string& s);
	void increment_total_sent_by(const int& num);
	void flag_the_timeout();
	void flag_that_we_should_close_early();

	const std::map<std::string, std::string>& get_cookies() const;
	void set_cookies(const std::string&);

	CgiHandler  cgi;

private:
	Request();

	std::string _request_str;
	std::vector<unsigned char>	_request_body;
	std::string _response;
	uint16_t	_port;
	in_addr_t	_host;
	int			_response_status;
	int			_total_sent;
	int			_content_length;	// refers to body
	bool		_flagged_as_chunked;
	bool		_done_reading_headers;
	bool		_should_close_early;
	int			_content_type_idx;  // content_types[n] || macros: TEXT_PLAIN, IMAGE_JPG
	std::vector<std::string> _content_type_params;
	int			_client_fd;
	bool		_keep_alive;
	bool		_timed_out;
	bool		_await_reconnection;
	std::vector<std::string> _accepted_types;
	std::multimap<float, std::string> _accepted_types_m;

	int			_method;
	int			_major_http_v;
	int			_minor_http_v;
	std::string _request_uri;

	std::map<std::string, std::string> _cookies;

	int			_cgi_status;
	std::string _cgi_job_id;
	std::string _cgi_output;

};
