# include "Log.hpp"

static void log(const std::string & message)
{
    if (DEBUG == 0)
        return ;
    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    std::cout << GREEN << "[" << now->tm_year + 1900 
    << "-" << now->tm_mon + 1 
    << "-" << now->tm_mday << " " 
    << now->tm_hour << ":" 
    << now->tm_min << ":" 
    << now->tm_sec << "] " 
    << "DEBUG: " << message 
    << WHITE << std::endl;
}