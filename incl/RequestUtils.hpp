#pragma once

#include <string>
#include <sstream>
#include <climits>
#include <vector>
#include "HttpHeaders.hpp"
#include "Exceptions.hpp"

#ifndef OK
# define OK 0
#endif

std::vector<std::string> split(const std::string& s, const std::string& delimiters);
int		webserv_atoi_set(const std::string& s, int& num);
int		set_http_v(const std::string& num, int& http_v);
bool	is_empty_crlf(std::string& line);
bool	is_lws(const char c);
void	check_stream(std::istringstream& stream);
int		get_http_header_idx(const std::string& s);


