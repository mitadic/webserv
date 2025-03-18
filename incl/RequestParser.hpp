#pragma once

#include <algorithm>
#include "Request.hpp"
#include "HttpHeaders.hpp"
#include "Exceptions.hpp"
#include "StatusCodes.hpp"
#include "ContentTypes.hpp"
#include "RequestUtils.hpp"

class RequestParser {
public:
	RequestParser();
	~RequestParser();
	RequestParser(const RequestParser&);

	void	parse_request_line(Request& req, std::string& line);
	void	parse_headers(Request& req, std::istringstream& stream, std::string& line);
	void	parse_body(Request& req, std::istringstream& stream, std::string& line);

	void	parse_header_line(Request& req, std::istringstream& stream, std::string& line);
	void	dispatch_header_parser(Request& req, const int legal_header_idx, std::string& header_val);


private:

	void	_parse_header_cache_control(Request&, std::string&);
	void	_parse_header_connection(Request&, std::string&);
	void	_parse_header_date(Request&, std::string&);
	void	_parse_header_pragma(Request&, std::string&);
	void	_parse_header_trailer(Request&, std::string&);
	void	_parse_header_transfer_encoding(Request&, std::string&);
	void	_parse_header_upgrade(Request&, std::string&);
	void	_parse_header_via(Request&, std::string&);
	void	_parse_header_warning(Request&, std::string&);

	void	_parse_header_accept(Request&, std::string&);
	void	_parse_header_accept_charset(Request&, std::string&);
	void	_parse_header_accept_encoding(Request&, std::string&);
	void	_parse_header_accept_language(Request&, std::string&);
	void	_parse_header_authorization(Request&, std::string&);
	void	_parse_header_expect(Request&, std::string&);
	void	_parse_header_from(Request&, std::string&);
	void	_parse_header_host(Request&, std::string&);
	void	_parse_header_if_match(Request&, std::string&);
	void	_parse_header_if_modified_since(Request&, std::string&);
	void	_parse_header_if_none_match(Request&, std::string&);
	void	_parse_header_if_range(Request&, std::string&);
	void	_parse_header_unmodified_since(Request&, std::string&);
	void	_parse_header_max_forwards(Request&, std::string&);
	void	_parse_header_proxy_authorization(Request&, std::string&);
	void	_parse_header_range(Request&, std::string&);
	void	_parse_header_referer(Request&, std::string&);
	void	_parse_header_te(Request&, std::string&);
	void	_parse_header_user_agent(Request&, std::string&);
	void	_parse_header_cookie(Request&, std::string&);

	void	_parse_header_allow(Request&, std::string&);
	void	_parse_header_content_encoding(Request&, std::string&);
	void	_parse_header_content_language(Request&, std::string&);
	void	_parse_header_content_length(Request&, std::string&);
	void	_parse_header_content_location(Request&, std::string&);
	void	_parse_header_content_md5(Request&, std::string&);
	void	_parse_header_content_range(Request&, std::string&);
	void	_parse_header_content_type(Request&, std::string&);
	void	_parse_header_expires(Request&, std::string&);
	void	_parse_header_last_modified(Request&, std::string&);

	// void	_parse_header_cookie(std::string&);
	// void	_parse_header_auth_scheme(std::string&);

};
