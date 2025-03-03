#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <climits>
#include <stdint.h>  // uint16_t

#include "HttpHeaders.hpp"
#include "Exceptions.hpp"
#include "Types.hpp"

#define SP " "
#define LWS_CHARS " \t"
#define UNINITIALIZED -1
#define HTTP_SEPARATORS "()<>@,;:\\\"/[]?={} \t"
#define DOTS_OR_DIGITS ".1234567890"
#define LOOPBACK_NUMERIC 2130706533
#define MAX_URI_LENGTH 4096  // NGINX default
#define MAX_CONTENT_LENGTH 10485760  

std::vector<std::string> split(const std::string& s, const std::string& delimiters);
void    trim_lws(std::string& s);
int		webserv_atoi_set(const std::string& s, int& num);
int		webserv_atouint16_set(const std::string& s, uint16_t& num);
int		set_http_v(const std::string& num, int& http_v);
bool	is_empty_crlf(std::string& line);
bool	is_lws(const char c);
bool    is_valid_ip_str(const std::string& s);
void    spin_through_leading_crlf(std::istringstream& stream, std::string& line);
void	check_stream_for_errors_or_eof(std::istringstream& stream);
void	check_stream_for_errors(std::istringstream& stream);
int		get_http_header_idx(const std::string& s);
bool	contains_non_digits(const std::string& s);
