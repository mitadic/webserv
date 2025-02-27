#pragma once

#include <exception>

class BadSyntaxException : public std::exception {

    private:
        BadSyntaxException();
        int error_code;
    
    public:
        BadSyntaxException(int code);
    
        const int code() const throw();
    
        // const char* what() const throw() {
        //     return "Bad syntax";
        // }
    };