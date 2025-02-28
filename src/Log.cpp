
# include "Log.hpp"
# include "Config.hpp"

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
}

void Log::log(std::vector<ServerBlock> & server_blocks)
{
    if (server_blocks.empty())
        return ;
    for (std::vector<ServerBlock>::const_iterator it = server_blocks.begin(); it != server_blocks.end(); ++it)
    {
        std::clog << std::endl;
        log("ServerBlock: ", INFO);
        std::clog << PURPLE;
        std::clog << "  port: " << it->port << std::endl;
        std::clog << "  host: " << Config::ft_inet_ntoa(*(in_addr_t *)&it->host) << std::endl;
        std::clog << "  max_client_body: " << it->max_client_body << std::endl;
        std::clog << "  error_pages: " << std::endl;
        for (std::map<int, std::string>::const_iterator it2 = it->error_pages.begin(); it2 != it->error_pages.end(); ++it2)
            std::clog << "    " << it2->first << " " << it2->second << std::endl;
        std::clog << "Locations:" << std::endl;
        for (std::vector<Location>::const_iterator it2 = it->locations.begin(); it2 != it->locations.end(); ++it2)
        {
            std::clog << "Location: " << it2->location << std::endl;
            std::clog << "  root: " << it2->root << std::endl;
            std::clog << "  index: " << it2->index << std::endl;
            std::clog << "  upload_location: " << it2->upload_location << std::endl;
            std::clog << "  upload_allowed: " << it2->upload_allowed << std::endl;
            std::clog << "  allowed_methods: ";
            std::clog << "  get: " << it2->get;
            std::clog << "  post: " << it2->post;
            std::clog << "  del: " << it2->del << std::endl;
            std::clog << "  autoindex: " << it2->autoindex << std::endl;
            std::clog << "  cgi_extensions: ";
            for (std::vector<std::string>::const_iterator it3 = it2->cgi_extensions.begin(); it3 != it2->cgi_extensions.end(); ++it3)
                std::clog << *it3 << " ";
            std::clog << std::endl;
            std::clog << "  redirect: ";
            std::clog << it2->redirect.first << " " << it2->redirect.second << std::endl;
        }
        std::clog << WHITE;
    }
}