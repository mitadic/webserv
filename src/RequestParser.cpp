#include "RequestParser.hpp"

RequestParser::RequestParser() {}

RequestParser::~RequestParser() {};

RequestParser::RequestParser(const RequestParser &oth)
{
	(void)oth;
}

// ###############################################

void RequestParser::_parse_header_cache_control(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_connection(Request &req, std::string &header_val)
{
	std::vector<std::string> values = split(header_val, ",");

	for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); it++)
	{
		trim_lws(*it);
		if (*it == "keep-alive")
			req._keep_alive = true;
		else if (*it == "close")
			req._keep_alive = false;
		// else if (*it == "Upgrade")
		// 	;
	}
}

void RequestParser::_parse_header_date(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_pragma(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_trailer(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_transfer_encoding(Request &req, std::string &header_val)
{
	std::vector<std::string> values = split(header_val, ",");
	for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); it++)
	{
		trim_lws(*it);
		if (req._flagged_as_chunked == true)
			throw RequestException(CODE_400); // no transfer-encodings allowed once "chunked" has been set
		if (*it == "chunked")
			req._flagged_as_chunked = true;
		else
		{
			; // placeholder
		}
	}
}

void RequestParser::_parse_header_upgrade(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_via(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_warning(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_accept(Request &req, std::string &header_val)
{
	std::vector<std::string> values = split(header_val, ",");
	for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); it++)
	{
		std::vector<std::string> specs = split(*it, ";"); // or use HTTP_SEPARATORS?
		trim_lws(specs[0]);
		if (specs[0].find_first_of("/") == std::string::npos ||
			specs[0].find_first_of("/") != specs[0].find_last_of("/") ||
			specs[0].find_first_of(HTTP_SEPARATORS_OTHER_THAN_FWDSLASH) != std::string::npos)
			throw RequestException(CODE_400);

		float quality_factor = 1.0f;

		for (std::vector<std::string>::iterator it_j = specs.begin() + 1; it_j != specs.end(); it_j++)
		{
			trim_lws(*it_j);
			if ((*it_j).size() < 3 || (*it_j)[0] != 'q' || (*it_j)[1] != '=') // Flag: make it case-insensitive
				continue;
			std::string q_value_string = (*it_j).substr(2);
			// if (contains_non_digits(q_value_string))
			// 	throw RequestException(CODE_400);

			char *endptr;
			quality_factor = std::strtof((*it_j).substr(2).c_str(), &endptr);
			if (endptr == q_value_string.c_str() || *endptr != '\0' ||
				quality_factor < 0.0f || quality_factor > 1.0f)
				throw RequestException(CODE_400);
		}

		req._accepted_types_m.insert(std::pair<float, std::string>(quality_factor, specs[0]));

		// actually, let's leave the validation for later, shall we?

		// for (int i = 0; i < CONTENT_TYPES_N; i++)
		// {
		// 	if (specs[0] == content_types[i])
		// 	{
		// 		req._accepted_types.push_back(*it);
		// 		break;
		// 	}
		// 	if (specs[0] == "text/*" || specs[0] == "application/*" || specs[0] == "image/*" || specs[0] == "*/*" )
		// 	{
		// 		req._accepted_types.push_back(specs[0]);
		// 		break;
		// 	}
		// }
	}

	// 	A more elaborate example is

	//        Accept: text/plain; q=0.5, text/html,
	//                text/x-dvi; q=0.8, text/x-c

	//    Verbally, this would be interpreted as "text/html and text/x-c are
	//    the preferred media types, but if they do not exist, then send the
	//    text/x-dvi entity, and if that does not exist, send the text/plain
	//    entity."

	//    Media ranges can be overridden by more specific media ranges or
	//    specific media types. If more than one media range applies to a given
	//    type, the most specific reference has precedence. For example,

	//        Accept: text/*, text/html, text/html;level=1, */*

	//    have the following precedence:

	//        1) text/html;level=1
	//        2) text/html
	//        3) text/*
	//        4) */*
}

void RequestParser::_parse_header_accept_charset(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_accept_encoding(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_accept_language(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_authorization(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_expect(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_from(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

/**
 * Host is a must-have and can't have repeat occurrences, so I'm a-throwin' exceptions
 * Notably, perhaps paradoxically, this is meant to confirm that the header has the info matching to the REALITY of the connection established on the SB
 */
void RequestParser::_parse_header_host(Request &req, std::string &header_val)
{
	// if (req._host)  // has already been set to something for this request. Not client, this request
	// 	throw RequestException(CODE_400);

	size_t start, end;
	uint16_t parsed_port = 0;
	in_addr_t parsed_host = 0;

	start = header_val.find_first_not_of(LWS_CHARS);
	if (start == std::string::npos)
		throw RequestException(CODE_400);
	end = header_val.find_first_of(":");
	if (end == std::string::npos) // if ':' not found
		end = header_val.size();
	else
	{
		std::string port_value_as_str = header_val.substr(end + 1);
		if (webserv_atouint16_set(port_value_as_str, parsed_port) != OK)
			throw RequestException(CODE_400);
	}

	std::string host_value_as_str = header_val.substr(start, end - start);
	if (host_value_as_str == "localhost")
		parsed_host = LOOPBACK_NUMERIC;
	else
	{
		if (!is_valid_ip_str(host_value_as_str))
			throw RequestException(CODE_400);
		std::vector<std::string> nums = split(host_value_as_str, ".");
		for (int i = 0; i < 4; i++)
			parsed_host |= (static_cast<in_addr_t>(std::atoi(nums[i].c_str())) << (8 * (3 - i)));
	}

	// parsed_port = htons(parsed_port);
	// parsed_host = htonl(parsed_host);
	if (parsed_port != req._port || parsed_host != req._host)
		throw RequestException(CODE_400);

	// validate whether "valid host IP" (non-private) later
}

void RequestParser::_parse_header_if_match(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_if_modified_since(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_if_none_match(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_if_range(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_unmodified_since(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_max_forwards(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_proxy_authorization(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_range(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_referer(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_te(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_user_agent(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_cookie(Request &req, std::string &header_val)
{
	// Log::log("Cookie header identified in the request: " + header_val, DEBUG);
	std::vector<std::string> kv_pairs = split(header_val, ",;");
	for (std::vector<std::string>::iterator it = kv_pairs.begin(); it != kv_pairs.end(); it++)
	{
		trim_lws(*it);
		std::vector<std::string> k_and_v = split(*it, "=");
		if (k_and_v.size() != 2)
			throw RequestException(CODE_400);
		req._cookies[k_and_v[0]] = k_and_v[1];
		// Log::log("Cookie added: " + k_and_v[0] + " : " + k_and_v[1], DEBUG);
	}
}

void RequestParser::_parse_header_allow(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_content_encoding(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_content_language(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_content_length(Request &req, std::string &header_val)
{
	if (req._content_length != UNINITIALIZED) // already initialized
		throw RequestException(CODE_400);
	if (webserv_atoi_set(header_val, req._content_length) != OK)
		throw RequestException(CODE_400);
	// if (req._content_length > MAX_CONTENT_LENGTH)
	// 	throw RequestException(CODE_400);
}

void RequestParser::_parse_header_content_location(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_content_md5(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_content_range(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_content_type(Request &req, std::string &header_val)
{
	if (req._content_type_idx != UNINITIALIZED)
		throw RequestException(CODE_400);
	std::vector<std::string> params = split(header_val, ";");
	const std::string header_defined_content_type = params[0];
	for (std::vector<std::string>::iterator it = params.begin() + 1; it != params.end(); it++)
	{
		trim_lws(*it);
		req._content_type_params.push_back(*it);
	}

	for (int i = 0; i < CONTENT_TYPES_N; i++)
	{
		if (content_types[i] == header_defined_content_type)
		{
			req._content_type_idx = i;
			return;
		}
	}
	throw RequestException(CODE_415);
}

void RequestParser::_parse_header_expires(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

void RequestParser::_parse_header_last_modified(Request &req, std::string &header_val)
{
	(void)req;
	(void)header_val;
}

// ###############################################

/* Early return if UNRECOGNIZED_HEADER. Else substr() the value trimmed away from leading LWS or trailing '\r' */
void RequestParser::dispatch_header_parser(Request &req, const int header_idx, std::string &header_val)
{
	if (header_idx == UNRECOGNIZED_HEADER)
	{
		// log?
		return;
	}

	// gotta do this, some headers only allow 1*DIGIT and no folded BS
	if (is_empty_crlf(header_val))
		return;
	size_t start = header_val.find_first_not_of(LWS_CHARS);
	size_t end = header_val.size(); // not the index, we actually need the size for the substr math
	if (*(header_val.rbegin()) == '\r')
		end--;
	std::string trimmed_val = header_val.substr(start, end - start);

	void (RequestParser::*header_parsers[HTTP_REQUEST_LEGAL_HEADERS_N])(Request &, std::string &) = {
		&RequestParser::_parse_header_cache_control,
		&RequestParser::_parse_header_connection,
		&RequestParser::_parse_header_date,
		&RequestParser::_parse_header_pragma,
		&RequestParser::_parse_header_trailer,
		&RequestParser::_parse_header_transfer_encoding,
		&RequestParser::_parse_header_upgrade,
		&RequestParser::_parse_header_via,
		&RequestParser::_parse_header_warning,
		&RequestParser::_parse_header_accept,
		&RequestParser::_parse_header_accept_charset,
		&RequestParser::_parse_header_accept_encoding,
		&RequestParser::_parse_header_accept_language,
		&RequestParser::_parse_header_authorization,
		&RequestParser::_parse_header_expect,
		&RequestParser::_parse_header_from,
		&RequestParser::_parse_header_host,
		&RequestParser::_parse_header_if_match,
		&RequestParser::_parse_header_if_modified_since,
		&RequestParser::_parse_header_if_none_match,
		&RequestParser::_parse_header_if_range,
		&RequestParser::_parse_header_unmodified_since,
		&RequestParser::_parse_header_max_forwards,
		&RequestParser::_parse_header_proxy_authorization,
		&RequestParser::_parse_header_range,
		&RequestParser::_parse_header_referer,
		&RequestParser::_parse_header_te,
		&RequestParser::_parse_header_user_agent,
		&RequestParser::_parse_header_cookie,
		&RequestParser::_parse_header_allow,
		&RequestParser::_parse_header_content_encoding,
		&RequestParser::_parse_header_content_language,
		&RequestParser::_parse_header_content_length,
		&RequestParser::_parse_header_content_location,
		&RequestParser::_parse_header_content_md5,
		&RequestParser::_parse_header_content_range,
		&RequestParser::_parse_header_content_type,
		&RequestParser::_parse_header_expires,
		&RequestParser::_parse_header_last_modified};
	(this->*header_parsers[header_idx])(req, trimmed_val);
}

// "GET /about.html HTTP/1.1
// Host: localhost:9991
// User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Accept-Language: en-GB,en;q=0.5
// Accept-Encoding: gzip, deflate, br, zstd
// DNT: 1
// Connection: keep-alive
// "

// Request       = Request-Line
//                         *(( general-header
//                          | request-header
//                          | entity-header ) CRLF)
//                         CRLF
//                         [ message-body ]

// Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

// message-header = field-name ":" [ field-value ]
//        field-name     = token
//        field-value    = *( field-content | LWS )
//        field-content  = <the OCTETs making up the field-value
//                         and consisting of either *TEXT or combinations
//                         of token, separators, and quoted-string>
// Note: LWS == linear white space

void RequestParser::parse_request_line(Request &req, std::string &line)
{
	std::vector<std::string> tokens = split(line, " ");
	if (tokens.size() != 3)
		throw RequestException(CODE_400);

	if (tokens[0] == "GET")
		req._method = GET;
	else if (tokens[0] == "POST")
		req._method = POST;
	else if (tokens[0] == "DELETE")
		req._method = DELETE;
	else
		throw RequestException(CODE_501);

	if (tokens[1][0] != '/')
		throw RequestException(CODE_400);
	if (tokens[1].size() > MAX_URI_LENGTH)
		throw RequestException(CODE_414);
	// split uri and query string
	size_t questionmark_pos = tokens[1].find_first_of('?');
	if (questionmark_pos == std::string::npos)
		req._request_uri = tokens[1];
	else
	{
		req._request_uri = tokens[1].substr(0, questionmark_pos);
		req._query_string = tokens[1].substr(questionmark_pos + 1);  // will produce "" if ? is at final pos
	}
	// check uri for '//' occurrences
	char prev = '\0';
	for (size_t i = 0; i < req._request_uri.size(); i++)
	{
		if (prev == '/' && req._request_uri[i] == '/')
			throw RequestException(CODE_400);
		prev = req._request_uri[i];
	}
	if (req._request_uri.find("coffee") != std::string::npos)
		throw RequestException(CODE_418);

	size_t dot = 0;
	if (tokens[2] == "undefined") // seen this in Mozilla
	{
		req._major_http_v = 1;
		req._minor_http_v = 1;
		return;
	}
	if (tokens[2].substr(0, 5) != "HTTP/")
		throw RequestException(CODE_400);
	dot = tokens[2].find_first_of(".", 5);
	if (dot == std::string::npos || dot == 5) // no '.' or begins with '.'
		throw RequestException(CODE_400);
	std::string num = tokens[2].substr(5, dot - 5);
	if (set_http_v(num, req._major_http_v) != OK)
		throw RequestException(CODE_400);
	num = tokens[2].substr(dot + 1);
	if (set_http_v(num, req._minor_http_v) != OK)
		throw RequestException(CODE_400);
}

/**
 * Continues reading through everything (robustly) until CRLF or EOF
 * Finally, if _content_length is UNINITIALIZED, set it to 0 for the arithmetics later on
 */
void RequestParser::parse_headers(Request &req, std::istringstream &stream, std::string &line)
{
	if (!std::getline(stream, line))
		check_stream_for_errors_or_eof(stream);

	while (!is_empty_crlf(line) && !stream.eof())
	{
		// Log::log("Parsing header line: " + line, DEBUG);
		parse_header_line(req, stream, line);
	}

	if (req._content_length == UNINITIALIZED)
		req._content_length = 0;
}

void RequestParser::parse_header_line(Request &req, std::istringstream &stream, std::string &line)
{
	size_t colon_pos = line.find_first_of(":");
	size_t lws_pos = line.find_first_of(HTTP_SEPARATORS);
	if (colon_pos == 0 || colon_pos == line.size() || lws_pos == 0 || lws_pos < colon_pos)
		throw RequestException(400);
	const std::string header_key = line.substr(0, colon_pos);
	std::string header_value = line.substr(colon_pos + 1);

	int header_idx = get_http_header_idx(header_key);
	dispatch_header_parser(req, header_idx, header_value);

	// while folded continuation, whether recognized header or no
	while (std::getline(stream, line) && !is_empty_crlf(line) && is_lws(line[0]))
		dispatch_header_parser(req, header_idx, line);
	if (!stream.eof())
		check_stream_for_errors(stream);
}
