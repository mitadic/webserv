#include <exception>

#include "ServerEngine.hpp"
#include "Config.hpp"
#include "Log.hpp"


volatile sig_atomic_t g_signal = 0; // forward declared in SignalHandling.hpp
t_log_level g_debug_level;			// forward declared in Log.hpp

void set_up_according_to_argv(int argc, char **argv, std::vector<ServerBlock>& server_blocks)
{
	int debug_in_argv = UNINITIALIZED;
	int conf_in_argv = UNINITIALIZED;

	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg.length() >= 5 && arg.substr(arg.length() - 5) == ".conf")
		{
			if (conf_in_argv != UNINITIALIZED)
				throw std::runtime_error("usage: ./webserv [config_file] [debug_level]");
			conf_in_argv = i;
			Config::parse_config(argv[i], server_blocks);
		}
		else
		{
			for (int l = 0; l < LOG_LEVELS_N; l++)
			{
				if (arg == log_levels[l])
				{
					if (debug_in_argv != UNINITIALIZED)
						throw std::runtime_error("usage: ./webserv [config_file] [debug_level]");
					g_debug_level = static_cast<t_log_level>(l);
					debug_in_argv = i;
					break;
				}
			}
		}
		if (debug_in_argv == UNINITIALIZED && conf_in_argv == UNINITIALIZED)
			throw std::runtime_error("usage: ./webserv [config_file] [debug_level]");
	}
	if (conf_in_argv == UNINITIALIZED)
		Config::parse_config(DEFAULT_CONF, server_blocks);
}

int main(int argc, char **argv)
{
	std::vector<ServerBlock> server_blocks;
	g_debug_level = INFO;

	try
	{
		if (argc == 1)
			Config::parse_config(DEFAULT_CONF, server_blocks);
		else if (argc > 3)
			throw std::runtime_error("usage: ./webserv [config_file] [debug_level]");
		else
			set_up_according_to_argv(argc, argv, server_blocks);
		Log::log("Server blocks are ready:", INFO);
		//Log::log(server_blocks);
	}
	catch (std::exception & e)
	{
		Log::log(std::string(e.what()), ERROR);
		return (1);
	}

	ServerEngine engine(server_blocks);

	engine.run();
}
