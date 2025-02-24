#ifndef LOG_HPP
# define LOG_HPP

# include <iostream>
# include <ctime>
# include <string>
# define DEBUG 1
# define GREEN "\033[32m"
# define WHITE "\033[0m"

class Log {
    public:
        static void log(const std::string & message);

        // ideas for future logging overloads:
        // static void log(const Request & request);
        // static void log(const Response & response);
        
};

#endif