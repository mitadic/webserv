
# include "Log.hpp"
# include "Config.hpp"
# include "ServerBlock.hpp"
# include "Location.hpp"
# include "Utils.hpp"

const char *log_levels[LOG_LEVELS_N] = {
	"SETUP",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"
};

void Log::log(const std::string message, t_log_level level)
{
    if (!DEBUGMODE || level < g_debug_level)
        return ;
    std::string color = WHITE;

	if (level == SETUP)
		color = BLUE;
    if (level == DEBUG)
        color = CYAN;
    if (level == INFO)
        color = GREEN;
    if (level == WARNING)
        color = PURPLE;
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
		case SETUP:
			std::clog << "SETUP: ";
			break;
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

void Log::log(std::vector<ServerBlock> & server_blocks, t_log_level level)
{
	if (!DEBUGMODE || level < g_debug_level)
        return ;
    if (server_blocks.empty())
    	return ;
    log("ServerBlocks: ", level);
    for (std::vector<ServerBlock>::const_iterator it = server_blocks.begin(); it != server_blocks.end(); ++it)
    {
        std::clog << BLUE;
        std::clog << *it;
        std::clog << WHITE;
    }
};

void Log::log(std::vector<ServerBlock> & server_blocks)
{
	if (server_blocks.empty())
		return ;
	for (std::vector<ServerBlock>::const_iterator it = server_blocks.begin(); it != server_blocks.end(); ++it)
	{
		std::ostringstream oss; oss << "Server running on: " + Utils::host_to_str(it->get_host()) << ":" << it->get_port();
		log(oss.str(), INFO);
	}
};
