#include "../incl/Exceptions.hpp"

BadSyntaxException::BadSyntaxException(int code) : error_code(code) {}

const int BadSyntaxException::code() const throw()
{
	return error_code;
}