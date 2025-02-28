#include "../incl/RequestUtils.hpp"

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

int	webserv_atoi_set(const std::string& s, int& num)
{
    if (s.empty())
		return 1;
	
	long long res = 0;
	std::string::const_iterator it = s.begin();
	for (; it != s.end() && *it == '0'; it++)  // RFC allows leading 0
		;
	if (it == s.end())
	{
		num = 0;
		return OK;
	}

	for (; it != s.end(); it++)
	{
		if (!std::isdigit(*it))  // RFC disallows +-, as well as any nondigits
			return 1;
		res = res * 10 + (*it - '0');
		if (res > INT_MAX)
			return 1;
	}
	num = res;
	return OK;
}

int set_http_v(const std::string& num, int& http_v)
{
	return webserv_atoi_set(num, http_v);
}

/* RFC 2616 permits leading newlines, so while \r\n and nothing else; EOF not permissible! */
void spin_through_leading_crlf(std::istringstream& stream, std::string& line)
{
	while (std::getline(stream, line) && is_empty_crlf(line))
		;
	check_stream(stream);
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
		throw RequestException(CODE_500);
	if (stream.eof())
		throw RequestException(CODE_400);
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