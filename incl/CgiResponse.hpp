#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP

# include <string>
# include "Exceptions.hpp"
# include "RequestUtils.hpp"
# include "Utils.hpp"

class CgiResponse {
public:
	CgiResponse();
	~CgiResponse();

	void parse_raw_cgi_output(const std::string& raw_cgi_output);
	void validate_and_format_headers(const std::string& cgi_generated_headers);
	void set_formatted_response(const std::string& body);

	void process_header_location(const std::string& value);
	void process_header_status(const std::string& value);
	void process_header_content_type(const std::string& value);
	void process_header_content_length(const std::string& value);

	std::string get_formatted_response();

private:

	std::string _status_code_and_msg;
	std::string _content_type;
	std::string _location;

	size_t _content_length;  // value ignored, we rely on our own length calc, but we throw 500 if multiple occurrences of Content-Length in raw_cgi_output

	std::string _formatted_headers;
	std::string _full_formatted_response;

	CgiResponse(const CgiResponse& oth);
	CgiResponse& operator=(const CgiResponse& oth);
};

#endif