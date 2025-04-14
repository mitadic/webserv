#ifndef LOG_HPP
# define LOG_HPP

# include <iostream>
# include <string>
# include <vector>
# include <ctime>
# include <arpa/inet.h>

# define GREEN "\033[32m"
# define WHITE "\033[0m"
# define PURPLE "\033[35m"
# define BLUE "\033[34m"
# define CYAN "\033[36m"
# define RED "\033[31m"
# define BROWN "\033[33m"
# define DEBUGMODE 1

class ServerBlock;
class Location;

typedef enum e_log_level
{
	SETUP,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    LOG_LEVELS_N
} t_log_level;

extern t_log_level g_debug_level;

extern const char *log_levels[LOG_LEVELS_N];

/**
 * @brief Class with static method overloads for logging debug messages
 */
class Log {
    public:
        static void log(const std::string message, t_log_level level);
        static void log(std::vector<ServerBlock> & server_blocks, t_log_level level);

        // ideas for future logging overloads:
        // static void log(const Request & request);
        // static void log(const Response & response);

};

#endif
