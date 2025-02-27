#include "../incl/Request.hpp"

Request::Request() :
		request(""),
		response(""),
		response_status(200),
		total_sent(0),
		method(0),
		client_fd(UNINITIALIZED),
		content_type_idx(UNINITIALIZED),
		timed_out(false),
		await_reconnection(false),
		keep_alive(true),
		cgi_status(NOT_CGI)
	{}

Request::~Request() {};

Request::Request(const Request& oth) : cgi()
{
	(void)oth;
}


void Request::reset()
{
	request.clear();
	response.clear();
	response_status = 200;
	total_sent = 0;
	method = 0;
	cgi_status = NOT_CGI;
	cgi_output.clear();
}

void Request::reset_client()
{
	request.clear();
	response.clear();
	response_status = 200;
	method = 0;
	client_fd = -1;
	total_sent = 0;
	cgi_status = NOT_CGI;
	cgi_output.clear();
}




// ###############################################

void Request::_parse_header_cache_control(std::string& header_val)
{}

void Request::_parse_header_connection(std::string& header_val)
{
	std::vector<std::string> values = split(header_val, " \t,");

	for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); it++)
	{
		if (*it == "keep-alive")
			keep_alive = true;
		else if (*it == "close")
			keep_alive = false;
		else if (*it == "Upgrade")
			;
	}
}

void Request::_parse_header_date(std::string& header_val)
{}

void Request::_parse_header_pragma(std::string& header_val)
{}

void Request::_parse_header_trailer(std::string& header_val)
{}

void Request::_parse_header_transfer_encoding(std::string& header_val)
{}

void Request::_parse_header_upgrade(std::string& header_val)
{}

void Request::_parse_header_via(std::string& header_val)
{}

void Request::_parse_header_warning(std::string& header_val)
{}

void Request::_parse_header_accept(std::string& header_val)
{}

void Request::_parse_header_accept_charset(std::string& header_val)
{}

void Request::_parse_header_accept_encoding(std::string& header_val)
{}

void Request::_parse_header_accept_language(std::string& header_val)
{}

void Request::_parse_header_authorization(std::string& header_val)
{}

void Request::_parse_header_expect(std::string& header_val)
{}

void Request::_parse_header_from(std::string& header_val)
{}

void Request::_parse_header_host(std::string& header_val)
{
	size_t start, end;
	start = header_val.find_first_not_of(LWS_CHARS);
	end = header_val.find_first_of(":");

	std::string trimmed_s = header_val.substr(start, end - start);
	if (trimmed_s == "127.0.0.1" || trimmed_s == "0.0.0.0" || trimmed_s == "localhost")
		host == trimmed_s;
	// port?
}

void Request::_parse_header_if_match(std::string& header_val)
{}

void Request::_parse_header_if_modified_since(std::string& header_val)
{}

void Request::_parse_header_if_none_match(std::string& header_val)
{}

void Request::_parse_header_if_range(std::string& header_val)
{}

void Request::_parse_header_unmodified_since(std::string& header_val)
{}

void Request::_parse_header_max_forwards(std::string& header_val)
{}

void Request::_parse_header_proxy_authorization(std::string& header_val)
{}

void Request::_parse_header_range(std::string& header_val)
{}

void Request::_parse_header_referer(std::string& header_val)
{}

void Request::_parse_header_te(std::string& header_val)
{}

void Request::_parse_header_user_agent(std::string& header_val)
{}

void Request::_parse_header_allow(std::string& header_val)
{}

void Request::_parse_header_content_encoding(std::string& header_val)
{}

void Request::_parse_header_content_language(std::string& header_val)
{}

void Request::_parse_header_content_length(std::string& header_val)
{}

void Request::_parse_header_content_location(std::string& header_val)
{}

void Request::_parse_header_content_md5(std::string& header_val)
{}

void Request::_parse_header_content_range(std::string& header_val)
{}

void Request::_parse_header_content_type(std::string& header_val)
{
	for (int i = 0; i < CONTENT_TYPES_N; i++)
	{
		if (content_types[i] == header_val)
		{
			content_type_idx = i;
			return;
		}
	}
	throw BadSyntaxException(CODE_415);
}

void Request::_parse_header_expires(std::string& header_val)
{}

void Request::_parse_header_last_modified(std::string& header_val)
{}

// ###############################################




void Request::dispatch_header_parser(const int header_idx, std::string& header_val)
{
	if (header_idx == UNRECOGNIZED_HEADER)
	{
		// log?
		return;
	}

	void    (Request::*header_parsers[HTTP_REQUEST_LEGAL_HEADERS_N])(std::string&) = {
		&Request::_parse_header_cache_control,
		&Request::_parse_header_connection,
		&Request::_parse_header_date,
		&Request::_parse_header_pragma,
		&Request::_parse_header_trailer,
		&Request::_parse_header_transfer_encoding,
		&Request::_parse_header_upgrade,
		&Request::_parse_header_via,
		&Request::_parse_header_warning,
		&Request::_parse_header_accept,
		&Request::_parse_header_accept_charset,
		&Request::_parse_header_accept_encoding,
		&Request::_parse_header_accept_language,
		&Request::_parse_header_authorization,
		&Request::_parse_header_expect,
		&Request::_parse_header_from,
		&Request::_parse_header_host,
		&Request::_parse_header_if_match,
		&Request::_parse_header_if_modified_since,
		&Request::_parse_header_if_none_match,
		&Request::_parse_header_if_range,
		&Request::_parse_header_unmodified_since,
		&Request::_parse_header_max_forwards,
		&Request::_parse_header_proxy_authorization,
		&Request::_parse_header_range,
		&Request::_parse_header_referer,
		&Request::_parse_header_te,
		&Request::_parse_header_user_agent,
		&Request::_parse_header_allow,
		&Request::_parse_header_content_encoding,
		&Request::_parse_header_content_language,
		&Request::_parse_header_content_length,
		&Request::_parse_header_content_location,
		&Request::_parse_header_content_md5,
		&Request::_parse_header_content_range,
		&Request::_parse_header_content_type,
		&Request::_parse_header_expires,
		&Request::_parse_header_last_modified
    };
	(this->*header_parsers[header_idx])(header_val);
}

/* Split at first of delimiters. If \r at the end of final token, pop it off */
std::vector<std::string> split(const std::string& s, const std::string& delimiters)
{
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = 0;

	while (start < s.size())
	{
		start = s.find_first_not_of(delimiters, end);	// start looking at 'end'
		if (start == std::string::npos)
			break;

		end = s.find_first_of(delimiters, start);		// start looking at 'start'
		tokens.push_back(s.substr(start, end - start));
	}

	if (!tokens.empty() && tokens.back().back() == '\r')
		tokens.back().pop_back();

	return tokens;
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

int set_http_v(std::string& num, int& http_v)
{
	if (num.empty())
		return 1;
	
	long long res = 0;
	std::string::iterator it = num.begin();
	for (; it != num.end() && *it == '0'; it++)  // RFC allows leading 0
		;
	if (it == num.end())
	{
		http_v = 0;
		return OK;
	}

	for (; it != num.end(); it++)
	{
		if (!std::isdigit(*it))  // RFC disallows +-, as well as any nondigits
			return 1;
		res = res * 10 + (*it - '0');
		if (res > INT_MAX)
			return 1;
	}
	http_v = res;
	return OK;
}

bool is_empty_crlf(std::string& line)
{
	// getline() trimmed the '\n'
	if (line.size() == 0 || (line.size() == 1 && line.back() == '\r'))
		return true;
	return false;
}

bool is_lws(const char c)
{
	if (c == 32 || c == 9)
		return true;
	return false;
}

void check_stream(std::istringstream& stream)
{
	if (stream.fail() || stream.bad())
		throw BadSyntaxException(500);
	if (stream.eof())
		throw BadSyntaxException(400);
}

int Request::parse_request_line(std::string& line)
{
	std::vector<std::string> tokens = split(line, " ");
	if (tokens.size() != 3)
		return 1;
	
	if (tokens[0] == "GET")
		method = 0b00000001;
	else if (tokens[0] == "POST")
		method = 0b00000010;
	else if (tokens[0] == "DELETE")
		method = 0b00000100;
	else
		return 1;

	request_location = tokens[1];

	size_t dot = 0;
	if (tokens[2].substr(0, 5) != "HTTP/")
		return 1;
	dot = tokens[2].find_first_of(".", 5);
	if (dot == std::string::npos || dot == 5)  // no '.' or begins with '.'
		return 1;
	std::string num = tokens[2].substr(5, dot - 5);
	if (set_http_v(num, major_http_v) != OK)
		return 1;
	num = tokens[2].substr(dot + 1);
	if (set_http_v(num, minor_http_v) != OK)
		return 1;
	
	return OK;
}

int get_http_header_idx(const std::string& s)
{
	for (int i = 0; i < HTTP_HEADERS_N; i++)
	{
		if (s == http_header_names[i])
			return i;
	}
	return UNRECOGNIZED_HEADER;
}

void Request::parse_header_line(std::istringstream& stream, std::string& line)
{
	size_t colon_pos = line.find_first_of(":");
	if (colon_pos == 0 || colon_pos == line.size() || line.find_first_of(HTTP_SEPARATORS) < colon_pos)
		throw BadSyntaxException(400);
	const std::string header_key = line.substr(0, colon_pos);
	std::string header_value = line.substr(colon_pos + 1);

	int header_idx = get_http_header_idx(header_key);
	dispatch_header_parser(header_idx, header_value);

	// while folded continuation, whether recognized header or no
	while (std::getline(stream, line) && !is_empty_crlf(line) && is_lws(line.front()))
		dispatch_header_parser(header_idx, line);
	check_stream(stream);
}

int Request::parse()
{
	std::istringstream stream(request);
	std::string line;

	// RFC 2616 permits leading newlines, so while \r\n and nothing else
	while (std::getline(stream, line) && is_empty_crlf(line))
		;
	check_stream(stream);

	// Parse the request-line
	if (parse_request_line(line) != OK)
		return 400;

	// if missing the min 'Host' or 1st char is indent, no good
	if (!std::getline(stream, line) || is_empty_crlf(line) || line.find_first_of(LWS_CHARS) == 0)
	{
		check_stream(stream);  // throw if (health issues | EOF)
		return 400;  // else 
	}

	// Parse the headers
	while (!is_empty_crlf(line))  // if empty CRLF we're moving on to the body	
		parse_header_line(stream, line);

	// if is GET and we've read anything past empty CRLF, that's illegal
	if (std::getline(stream, line) && method & 0b00000001)
		return 400;  // nope, actually no

	// read body (if any) until another CRLF, then we're done
	while (std::getline(stream, line) && !is_empty_crlf(line))
	{
		;
	}
}