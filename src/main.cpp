#include <exception>

#include "ServerEngine.hpp"
#include "Config.hpp"
#include "Log.hpp"


volatile sig_atomic_t g_signal = 0;  // forward declared in SignalHandling.hpp


int main(int argc, char **argv)
{
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
		//Log::log(server_blocks);
	}
	catch (std::exception & e)
	{
		Log::log("Error: " + std::string(e.what()), ERROR);
		return (1);
	}

	ServerEngine engine(server_blocks);

	engine.run();
}
