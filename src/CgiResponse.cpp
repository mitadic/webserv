#include "CgiResponse.hpp"

CgiResponse::CgiResponse() : _content_length(0) {}
CgiResponse::~CgiResponse() {}
CgiResponse::CgiResponse(const CgiResponse&) {}  // defunct, private
CgiResponse& CgiResponse::operator=(const CgiResponse&) { return *this; }  // defunct, private


void CgiResponse::process_header_location(const std::string& value)
{
	if (!_location.empty())
	{
		Log::log("Bad CGI output: Had more than one instances of 'Location' header", WARNING);
		throw RequestException(CODE_500);
	}
	_location = value;
	_formatted_headers += "Location: " + _location;
}

void CgiResponse::process_header_status(const std::string& value)
{
	std::vector<std::string> code_and_reason = split(value, " ");
	const int code_input = std::strtod(code_and_reason[0].c_str(), NULL);
	for (int i = 0; i < STATUS_CODES_N; i++)
	{
		if (code_input == status_code_values[i])
		{
			if (!_status_code_and_msg.empty())
			{
				Log::log("Bad CGI output: Had more than one instances of 'Status' header", WARNING);
				throw RequestException(CODE_500);
			}
			_status_code_and_msg = status_messages[i];
			_formatted_headers += std::string("Status: ") + _status_code_and_msg;
			break;
		}
	}
	if (_status_code_and_msg.empty())
	{
		Log::log("Bad CGI output: Had a 'Status' header but did not include a valid status code", WARNING);
		throw RequestException(CODE_500);
	}
}

void CgiResponse::process_header_content_type(const std::string& value)
{
	if (!_content_type.empty())
	{
		Log::log("Bad CGI output: Had more than one instances of 'Content-Type' header", WARNING);
		throw RequestException(CODE_500);
	}
	_content_type = value;
	_formatted_headers += "Content-Type: " + _content_type;
}

void CgiResponse::process_header_content_length(const std::string& value)
{
	if (_content_length)
	{
		Log::log("Bad CGI output: Content-Length header specified more than once", WARNING);
		throw RequestException(CODE_500);
	}
	_content_length = std::strtod(value.c_str(), NULL);
}

/* Inspect cgi_generated_headers chiefly for the 3 'CGI-headers', validate, and create formatted_headers */
void CgiResponse::validate_and_format_headers(const std::string& cgi_generated_headers)
{
	std::istringstream headers_iss(cgi_generated_headers);
	std::string line;
	while (std::getline(headers_iss, line))
	{
		std::vector<std::string> name_and_val = split(line, ":");
		if (name_and_val.size() != 2)
		{
			Log::log("Bad CGI output: A header didn't feature exactly one of ':'", WARNING);
			throw RequestException(CODE_500);
		}
		trim_lws(name_and_val[1]);

		if (Utils::is_ci_equal_str(name_and_val[0], "content-type"))  // not inspecting or validating the type, only confirming that no multiple occurrences
		{
			process_header_content_type(name_and_val[1]);
		}
		else if (Utils::is_ci_equal_str(name_and_val[0], "location"))
		{
			process_header_location(name_and_val[1]);
		}
		else if (Utils::is_ci_equal_str(name_and_val[0], "status"))
		{
			process_header_status(name_and_val[1]);
		}
		else if (Utils::is_ci_equal_str(name_and_val[0], "content-length"))
		{
			process_header_content_length(name_and_val[1]);
		}
		else if (Utils::is_ci_equal_str(name_and_val[0], "authorization"))
		{
			continue;
		}
		// include any other headers as they are, without validation, see 'generic-field' in RFC 3875, not server's responsibility
		else
			_formatted_headers += name_and_val[0] + std::string(": ") + name_and_val[1];

		// apply to any added header indiscriminately
		_formatted_headers += std::string("\r\n");
	}
	if (_content_type.empty() && _location.empty() && _status_code_and_msg.empty())
	{
		Log::log("Bad CGI output: did not include 1*(Content-Type | Location | Status)", WARNING);
		throw RequestException(CODE_500);
	}

	if (!_location.empty() && _formatted_headers.find_first_of("\r\n") != _formatted_headers.size() - 2)  // if any header other than Location
	{
		Log::log("Bad CGI output: specified Location but then had other headers which is explicitly forbidden", WARNING);
		throw RequestException(CODE_500);
	}
	else
	{
		_status_code_and_msg = status_messages[CODE_302];
		_formatted_headers += _status_code_and_msg + std::string("\r\n");
	}
}

void CgiResponse::set_formatted_response(const std::string& body)
{
	std::ostringstream response;
	response << "HTTP/1.1 " << (_status_code_and_msg.empty() ? "200 OK" : _status_code_and_msg) << "\r\n"
				 << _formatted_headers
				 << "Content-Length: " << body.length() << "\r\n"
				 << "\r\n"
				 << body;

	_full_formatted_response = response.str();
}

/* Get valid, complete and fully formatted HTTP response as the 200 outcome of a CGI request */
std::string CgiResponse::get_formatted_response()
{
	return _full_formatted_response;
}

/* Parse the raw CGI output, validate, and populate the CgiResponse attributes */
void CgiResponse::parse_raw_cgi_output(const std::string& raw_cgi_output)
{
	// separate headers from body.
	size_t headers_end_pos = raw_cgi_output.find("\r\n\r\n");
	size_t body_start_pos = 0;
	if (headers_end_pos == std::string::npos)
	{
		Log::log("Bad CGI output: did not include headers (mandatory 1*(Content-Type | Location | Status))", WARNING);
		throw RequestException(CODE_500);
	}
	body_start_pos = headers_end_pos + 4;
	std::string cgi_generated_headers = raw_cgi_output.substr(0, headers_end_pos + 2);
	std::string body = raw_cgi_output.substr(body_start_pos) + std::string("\r\n");

	validate_and_format_headers(cgi_generated_headers);
	set_formatted_response(body);
}
