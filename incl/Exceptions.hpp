#pragma once

#include <exception>
#include "StatusCodes.hpp"

class BadSyntaxException : public std::exception {

    private:
        BadSyntaxException();
        int error_code;
    
    public:
        BadSyntaxException(int code);
    
        const int code() const throw();
        const char* what() const throw();
    };