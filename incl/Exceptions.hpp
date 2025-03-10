#pragma once

#include <exception>
#include "StatusCodes.hpp"

class RequestException : public std::exception {

    private:
        RequestException();
        int error_code;

    public:
        RequestException(int code);

        int code() const throw();
        const char* what() const throw();
    };
