
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>
# include <map>
# include <cstring>
# include <sstream>
# include <fstream>
# include <arpa/inet.h>
# include <limits>

# include "Log.hpp"
# include "ServerBlock.hpp"

# define DEFAULT_CONF "conf/default.conf"

namespace Config {
        // Parsing
        void parse_config(const std::string & filename, std::vector<ServerBlock> & server_blocks);
        void parse_server_block(ServerBlock & block, std::stringstream & file, std::string & line);
        void parse_server_block_directives(std::string & line, ServerBlock & block, std::stringstream & content);
        void parse_location(std::string & line, Location & block, std::stringstream & content);
        void parse_location_block_directives(std::string & line, Location & block, std::stringstream & content);
        void validate_blocks(std::vector<ServerBlock> & server_blocks);
        std::stringstream load_file(const std::string & filename);
        
        // Utils
        std::string trim(const std::string & str);
        int has_only_digits(char *str);
        in_addr_t ft_inet(std::string host);
        std::string ft_inet_ntoa(in_addr_t ip);

        // Location block utils
        std::string check_location_prefix(Location & block, std::string & value);
        bool same_prefix(const Location & a, const Location & b);
        bool compare_prefix(const Location & a, const Location & b);

        // Server block utils
        void parse_client_body(ServerBlock & block, std::string & value);
        void parse_error_page(ServerBlock & block, std::string & value);
        void parse_host(ServerBlock & block, std::string & value);
        void parse_port(ServerBlock & block, std::string & value);   
};

#endif