
#include "ServerBlock.hpp"

ServerBlock::ServerBlock() : port(-1), host(inet_addr("255.255.255.255")), max_client_body(0) {};