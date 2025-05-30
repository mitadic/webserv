#include "RequestUtils.hpp"

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

	if (!tokens.empty() && *(tokens.back().rbegin()) == '\r')
		tokens.back() = tokens.back().substr(0, tokens.back().size() - 1);  // string::pop_back() in 98

	return tokens;
}

/* Trim any leading or trailing LWS */
void trim_lws(std::string& s)
{
	size_t start = s.find_first_not_of(LWS_CHARS);
	size_t end = s.find_last_not_of(LWS_CHARS);

	if (start == std::string::npos || end == std::string::npos)
	{
		s.clear();
		return;
	}
	if (start == 0 && end == s.size() - 1)
		return;

	s = s.substr(start, end - start + 1);
}

/* Wrapper for webserv_atoi_set() */
int set_http_v(const std::string& num, int& http_v)
{
	return webserv_atoi_set(num, http_v);
}

/* RFC 2616 permits leading newlines, so while \r\n and nothing else; EOF not permissible! */
void spin_through_leading_crlf(std::istringstream& stream, std::string& line)
{
	while (std::getline(stream, line) && is_empty_crlf(line))
		;
	check_stream_for_errors_or_eof(stream);
}

/* Check whether line is empty CRLF (or LF, as specified in the RFC) */
bool is_empty_crlf(std::string& line)
{
	// getline() trimmed the '\n'
	if (line.size() == 0 || (line.size() == 1 && *line.rbegin() == '\r'))
		return true;
	return false;
}

bool is_lws(const char c)
{
	if (c == 32 || c == 9)
		return true;
	return false;
}

/* Helper for the std::all_of (below) */
bool is_digit_or_dot(char c)
{
	if (std::isdigit(c) || c == '.')
		return true;
	return false;
}

bool is_valid_ip_str(const std::string& s)
{
	if (s.find_first_not_of(DOTS_OR_DIGITS) != std::string::npos)
		return false;

	std::vector<std::string> nums = split(s, ".");
	if (nums.size() != 4)
		return false;

	for (int i = 0; i < 4; i++)
	{
		int num;
		if (webserv_atoi_set(nums[i], num) != OK || num > 255)
			return false;
	}
	return true;
}

void check_stream_for_errors_or_eof(std::istringstream& stream)
{
	if (stream.eof())
		throw RequestException(CODE_400);
	if (stream.fail() || stream.bad())
	{
		Log::log("Stream error", ERROR);
		throw RequestException(CODE_500);
	}
}

void check_stream_for_errors(std::istringstream& stream)
{
	if (stream.fail() || stream.bad())
		throw RequestException(CODE_500);
}

/* Consult http_request_legal_headers */
int get_http_header_idx(const std::string& s)
{
	for (int i = 0; i < HTTP_REQUEST_LEGAL_HEADERS_N; i++)
	{
		if (s == http_request_legal_headers[i])
			return i;
	}
	return UNRECOGNIZED_HEADER;
}

bool contains_non_digits(const std::string& s)
{
	for (std::string::const_iterator it = s.begin(); it != s.end(); it++)
	{
		if (!std::isdigit(*it))
			return true;
	}
	return false;
}

int match_code(int status_code)
{
	int i = 0;
	while (i < STATUS_CODES_N)
	{
		if (status_code_values[i] == status_code)
			return (i);
		i++;
	}
	return (-1);
}
