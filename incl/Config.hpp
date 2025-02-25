
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>
# include <map>
# include <cstring>
# include <sstream>
# include <fstream>
# include <arpa/inet.h>

# include "Log.hpp"
# include "ServerBlock.hpp"

# define DEFAULT_CONF "conf/default.conf"

namespace Config {
        void parse_config(const std::string & filename, std::vector<ServerBlock> & server_blocks);
        void parse_server_block(ServerBlock & block, std::stringstream & file, std::string & line);
        std::stringstream load_file(const std::string & filename);
        std::string trim(const std::string & str);
        std::string replace_tabs_with_spaces(std::string & line);
};


#endif