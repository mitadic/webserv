
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>
# include <map>
# include <cstring>
# include <sstream>

# include "ServerBlock.hpp"

class Config {
    public:
        //std::string host;
        static std::vector<ServerBlock> parse_config(std::string filename);

    private:
        std::stringstream _input;
};

#endif