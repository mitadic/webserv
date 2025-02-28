#include "ServerEngine.hpp"
#include "../incl/ServerBlock.hpp"
#include "../incl/Config.hpp"
#include "../incl/Log.hpp"


volatile std::sig_atomic_t g_signal = 0;  // declared in ServerEngine.hpp


int main(int argc, char **argv) {

    ServerEngine engine;
    std::vector<ServerBlock> server_blocks;

    try
    {
        if (argc == 1)
            Config::parse_config(DEFAULT_CONF, server_blocks);
        else if (argc == 2)
            Config::parse_config(argv[1], server_blocks);
        else
            throw std::runtime_error("usage: ./webserv [config_file]");
        Log::log("Server blocks are ready:", INFO);
        Log::log(server_blocks);
        //engine.run();
    }
    catch (std::exception & e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return (1);
    }

}