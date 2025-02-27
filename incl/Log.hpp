#ifndef LOG_HPP
# define LOG_HPP

# include "ServerBlock.hpp"
# include <iostream>
# include <ctime>
# include <string>
# include <vector>
# include <arpa/inet.h>
# define GREEN "\033[32m"
# define WHITE "\033[0m"
# define PURPLE "\033[35m"
# define BLUE "\033[34m"
# define MAGENTA "\033[35m"
# define RED "\033[31m"
# define DEBUGMODE 1

typedef enum e_log_level
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
} t_log_level;

/**
 * @brief Class with static method overloads for logging debug messages
 */
class Log {
    public:
        static void log(const std::string & message, t_log_level level);
        static void log(std::vector<ServerBlock> & server_blocks);

        // ideas for future logging overloads:
        // static void log(const Request & request);
        // static void log(const Response & response);
        
};

#endif