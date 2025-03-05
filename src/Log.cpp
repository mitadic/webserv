
# include "Log.hpp"
# include "Config.hpp"
# include "ServerBlock.hpp"
# include "Location.hpp"

void Log::log(const std::string & message, t_log_level level)
{
    if (!DEBUGMODE)
        return ;
    std::string color = WHITE;

    if (level == DEBUG)
        color = BLUE;
    if (level == INFO)
        color = GREEN;
    if (level == WARNING)
        color = MAGENTA;
    if (level == ERROR)
        color = RED;

    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    std::clog << color << "[" << now->tm_year + 1900
    << "-" << now->tm_mon + 1
    << "-" << now->tm_mday << " "
    << now->tm_hour << ":"
    << now->tm_min << ":"
    << now->tm_sec << "] ";

    switch (level)
    {
        case DEBUG:
            std::clog << "DEBUG: ";
            break;
        case INFO:
            std::clog << "INFO: ";
            break;
        case WARNING:
            std::clog << "WARNING: ";
            break;
        case ERROR:
            std::clog << "ERROR: ";
            break;
        default:
            break;
    }

    std::clog << message
    << WHITE << std::endl;
    std::clog.flush();
};

void Log::log(std::vector<ServerBlock> & server_blocks)
{
    if (server_blocks.empty())
    return ;
    log("ServerBlocks: ", INFO);
    for (std::vector<ServerBlock>::const_iterator it = server_blocks.begin(); it != server_blocks.end(); ++it)
    {
        std::clog << PURPLE;
        std::clog << *it;
        std::clog << WHITE;
    }
};
