#pragma once

#include <sstream>
#include <climits>
#include "CgiHandler.hpp"
#include "HttpHeaders.hpp"
#include "Exceptions.hpp"
#include "StatusCodes.hpp"
#include "ContentTypes.hpp"
#include "RequestUtils.hpp"

#define SP " "
#define LWS_CHARS " \t"
#define UNINITIALIZED -1
#define HTTP_SEPARATORS "()<>@,;:\\\"/[]?={} \t"


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
	void	dispatch_header_parser(const int legal_header_idx, std::string& header_val);


	std::string host;
	std::string request;
	std::string	request_body;
    std::string response;
	int			response_status;
	int			total_sent;
	int			content_length;  // refers to body
	bool		chunked;
	int			content_type_idx;
	int         client_fd;
	bool		timed_out;
	bool		await_reconnection;
	std::vector<std::string> accepted_types;

	char		method;			// GET POST DELETE
	int			major_http_v;
	int			minor_http_v;
	std::string request_location;

	bool		keep_alive;		// default: true

	int			cgi_status;
	CgiHandler  cgi;
	std::string cgi_job_id;
	std::string cgi_output;

private:

	void	_parse_header_cache_control(std::string&);
	void	_parse_header_connection(std::string&);
	void	_parse_header_date(std::string&);
	void	_parse_header_pragma(std::string&);
	void	_parse_header_trailer(std::string&);
	void	_parse_header_transfer_encoding(std::string&);
	void	_parse_header_upgrade(std::string&);
	void	_parse_header_via(std::string&);
	void	_parse_header_warning(std::string&);

	void	_parse_header_accept(std::string&);
	void	_parse_header_accept_charset(std::string&);
	void	_parse_header_accept_encoding(std::string&);
	void	_parse_header_accept_language(std::string&);
	void	_parse_header_authorization(std::string&);
	void	_parse_header_expect(std::string&);
	void	_parse_header_from(std::string&);
	void	_parse_header_host(std::string&);
	void	_parse_header_if_match(std::string&);
	void	_parse_header_if_modified_since(std::string&);
	void	_parse_header_if_none_match(std::string&);
	void	_parse_header_if_range(std::string&);
	void	_parse_header_unmodified_since(std::string&);
	void	_parse_header_max_forwards(std::string&);
	void	_parse_header_proxy_authorization(std::string&);
	void	_parse_header_range(std::string&);
	void	_parse_header_referer(std::string&);
	void	_parse_header_te(std::string&);
	void	_parse_header_user_agent(std::string&);

	void	_parse_header_allow(std::string&);
	void	_parse_header_content_encoding(std::string&);
	void	_parse_header_content_language(std::string&);
	void	_parse_header_content_length(std::string&);
	void	_parse_header_content_location(std::string&);
	void	_parse_header_content_md5(std::string&);
	void	_parse_header_content_range(std::string&);
	void	_parse_header_content_type(std::string&);
	void	_parse_header_expires(std::string&);
	void	_parse_header_last_modified(std::string&);

	// void	_parse_header_cookie(std::string&);
	// void	_parse_header_auth_scheme(std::string&);

};