#include "../incl/StatusCodes.hpp"

const char *status_messages[STATUS_CODES_N] =
{
    "Continue",  // 100
    "Switching Protocols",
    "OK",  // 200
    "Created",
    "Accepted",
    "Non-Authoritative Information",
    "No Content",
    "Reset Content",
    "Partial Content",
    "Multiple Choices",  // 300
    "Moved Permanently",
    "Found",
    "See Other",
    "Not Modified",
    "Use Proxy",
    "Switch Proxy",  // unused in RFC 2616
    "Temporary Redirect",
    "Bad Request",  // 400
    "Unauthorized",
    "Payment Required",
    "Forbidden",
    "Not Found",
    "Method Not Allowed",
    "Not Acceptable",
    "Proxy Authentication Required",
    "Request Timeout",
    "Conflict",
    "Gone",
    "Length Required",
    "Precondition Failed",
    "Request Entity Too Large",
    "Request-URI Too Long",
    "Unsupported Media Type",
    "Requested Range Not Satisfiable",
    "Expectation Failed",
    "Internal Server Error",  // 500
    "Not Implemented",
    "Bad Gateway",
    "Service Unavailable",
    "Gateway Timeout",
    "HTTP Version Not Supported"
};