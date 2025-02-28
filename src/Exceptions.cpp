#include "../incl/Exceptions.hpp"


RequestException::RequestException(int code) : error_code(code) {}


const int RequestException::code() const throw()
{
	return error_code;
}


const char* RequestException::what() const throw()
{
	return status_messages[error_code];
}