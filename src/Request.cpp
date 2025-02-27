#include "../incl/Request.hpp"

Request::Request() :
		request(""),
		response(""),
		response_status(200),
		total_sent(0),
		method(0),
		client_fd(-1),
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
	return -1;
}

void Request::parse_header_line(std::istringstream& stream, std::string& line)
{
	size_t colon_pos = line.find_first_of(":");
	if (colon_pos == 0 || colon_pos == line.size())
		throw BadSyntaxException(400);
	std::string header = line.substr(0, colon_pos);

	int idx = get_http_header_idx(header);
		;  // call function to deal with specific header line...? and also in the follow-up loop?

	// while folded continuation
	while (std::getline(stream, line) && !is_empty_crlf(line) && is_lws(line.front()))
	{

	}
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

	while (std::getline(stream, line))
	{
		if (is_empty_crlf(line))
			return 400;  // we're in the body, no empty CRLF permitted
	}
}