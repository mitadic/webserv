
#include "ServerBlock.hpp"
#include "Config.hpp"

ServerBlock::ServerBlock() : port(-1), max_client_body(0) {
    host = Config::ft_inet("255.255.255.255");
};