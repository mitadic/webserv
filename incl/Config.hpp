
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>
# include <map>
# include <cstring>
# include <sstream>

# include "ServerBlock.hpp"

# define DEFAULT_CONF "conf/default.conf"

class Config {
    public:
        //std::string host;
        static void parse_config(std::string filename, std::vector<ServerBlock> & server_blocks);
        //void        parse_server_block(ServerBlock & block, std::ifstream &file);
        
        private:
        std::stringstream   _input;
    };

#endif