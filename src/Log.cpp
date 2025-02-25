
# include "Log.hpp"

void Log::log(const std::string & message)
{
    if (DEBUG == 0)
        return ;
    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    std::clog << GREEN << "[" << now->tm_year + 1900 
    << "-" << now->tm_mon + 1 
    << "-" << now->tm_mday << " " 
    << now->tm_hour << ":" 
    << now->tm_min << ":" 
    << now->tm_sec << "] " 
    << "DEBUG: " << message 
    << WHITE << std::endl;
    std::clog.flush();
}

void Log::log(std::vector<ServerBlock> & server_blocks)
{
    if (server_blocks.empty())
        return ;
    for (std::vector<ServerBlock>::const_iterator it = server_blocks.begin(); it != server_blocks.end(); ++it)
    {
        std::clog << PURPLE << "ServerBlock: " << std::endl;
        std::clog << "port: " << it->port << std::endl;
        std::clog << "host: " << inet_ntoa(*(in_addr *)&it->host) << std::endl;
        std::clog << "max_client_body: " << it->max_client_body << std::endl;
        std::clog << "locations: " << std::endl;
        for (std::vector<Location>::const_iterator it2 = it->locations.begin(); it2 != it->locations.end(); ++it2)
        {
            std::clog << "Location: " << it2->location << std::endl;
            std::clog << "root: " << it2->root << std::endl;
            std::clog << "index: " << it2->index << std::endl;
            std::clog << "upload_location: " << it2->upload_location << std::endl;
            std::clog << "upload_allowed: " << it2->upload_allowed << std::endl;
            std::clog << "get: " << it2->get << std::endl;
            std::clog << "post: " << it2->post << std::endl;
            std::clog << "del: " << it2->del << std::endl;
            std::clog << "autoindex: " << it2->autoindex << std::endl;
            std::clog << "cgi_extensions: " << std::endl;
            for (std::vector<std::string>::const_iterator it3 = it2->cgi_extensions.begin(); it3 != it2->cgi_extensions.end(); ++it3)
                std::clog << *it3 << std::endl;
            std::clog << "redirect: " << std::endl;
            for (std::map<int, std::string>::const_iterator it3 = it2->redirect.begin(); it3 != it2->redirect.end(); ++it3)
                std::clog << it3->first << " -> " << it3->second << std::endl;
        }
        std::clog << WHITE;
    }
}