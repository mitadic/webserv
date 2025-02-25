#ifndef LOG_HPP
# define LOG_HPP

# include "ServerBlock.hpp"
# include <iostream>
# include <ctime>
# include <string>
# include <vector>
# include <arpa/inet.h>
# define DEBUG 1
# define GREEN "\033[32m"
# define WHITE "\033[0m"
# define PURPLE "\033[35m"

/**
 * @brief Class with static method overloads for logging debug messages
 */
class Log {
    public:
        static void log(const std::string & message);
        static void log(std::vector<ServerBlock> & server_blocks);


        // ideas for future logging overloads:
        // static void log(const Request & request);
        // static void log(const Response & response);
        
};

#endif