
#include "Config.hpp"

/**
 * @brief Loads (the config) file into a stringstream
 */
void Config::load_file(const std::string & filename, std::stringstream& content)
{
	std::ifstream       file(filename.c_str());
	
	if (filename.substr(filename.find_last_of(".") + 1) != "conf")
		throw std::runtime_error("*.conf file extension required");
	if (!file.is_open())
		throw std::runtime_error("couldn't open config file");

	// content << file.rdbuf();
	char buffer[1024];
	while (file.read(buffer, sizeof(buffer)))
		content.write(buffer, file.gcount());
	content.write(buffer, file.gcount());

	file.close();
	Log::log("loaded config file: " + filename, INFO);
}

/**
 * @brief Parses the config file into a vector of server blocks
 */
void Config::parse_config(const std::string & filename, std::vector<ServerBlock> & server_blocks)
{
	std::string         line;
	std::stringstream   content;

	load_file(filename, content);
	while (getline(content, line))
	{
		line = trim(line);
		if (line.empty()) // skip empty lines
			continue ;
		if (line == "server {")
		{
			Log::log("new server block", DEBUG);
			ServerBlock server_block;
			parse_server_block(server_block, content, line);
			server_blocks.push_back(server_block);
		}
		else
			throw std::runtime_error("in config file: unexpected line");
	}
	validate_blocks(server_blocks);
	// iterate through serverblocks and remove double entries (same host and port)
}

/**
 * @brief Loops through server blocks and checks for missing directives,
 * for same host-port combinations and for same prefixes in location blocks
 */
void    Config::validate_blocks(std::vector<ServerBlock> & server_blocks)
{
	Log::log("Before validation:", DEBUG);
	Log::log(server_blocks);
	
	if (server_blocks.empty())
		throw std::runtime_error("Empty vector of server blocks");

	std::vector<ServerBlock>::iterator server_it;
	std::map<int, in_addr_t> host_port_combo;
	for (server_it = server_blocks.begin(); server_it != server_blocks.end(); ++server_it)
	{
		if (server_it->get_port() == -1 || server_it->get_host() == ft_inet("255.255.255.255") || server_it->get_max_client_body() == 0)
			throw std::runtime_error("missing directive 'listen', 'host' or 'client_max_body_size' inside server block");
		if (host_port_combo.find(server_it->get_port()) != host_port_combo.end())
		{
			if (host_port_combo[server_it->get_port()] == server_it->get_host())
			{
				Log::log("same host-port combination, removing server block...", WARNING);
				server_blocks.erase(server_it); // remove server block from blocks
				server_it--; // adjust iterator
			}
		}
		else 
			host_port_combo[server_it->get_port()] = server_it->get_host();
		if (server_it->get_locations().empty())
			continue ;
		server_it->validate_locations();
	}
}

/**
 * @brief Parses one server {...} part of the config file into a server block
 */
void Config::parse_server_block(ServerBlock & block, std::stringstream & content, std::string & line)
{
	Log::log("inside parse server block", DEBUG);
	while (getline(content, line))
	{
		line = trim(line); // remove abundant whitespaces and comments
		if (line.empty()) // skip empty lines
			continue ;

		if (line == "}") // end of server block
			return ;
		else if (line[line.size() - 1] == ';' 
			|| (line.find("location") != std::string::npos && line[line.size() - 1] == '{')) // remove ';'
		{
			line = line.substr(0, line.size() - 1);
			line = trim(line);
		}   
		else
			throw std::runtime_error("in server block: missing semicolon: " + line);
		parse_server_block_directives(line, block, content);
	}
}

/**
 * @brief Parses the directives inside a server block e.g. listen, error_page
 */
void Config::parse_server_block_directives(std::string & line, ServerBlock & block, std::stringstream & content)
{
	std::string         directive, value;
	std::stringstream   ss(line);
	if (getline(ss, directive, ' ') && getline(ss, value))
	{
		if (directive == "listen")
			block.set_port(value);
		else if (directive == "host")
			block.set_host(value);
		else if (directive == "error_page")
			block.add_error_page(value);
		else if (directive == "client_max_body_size")
		   block.set_max_client_body(value);
		else if (directive == "location")
		{
			Log::log("new location inside server block", DEBUG);
			Location location;
			parse_location(value, location, content);
			block.add_location(location);
		}
		else
			throw std::runtime_error("in sever block: unknown directive: " + directive);
	}
	else
		throw std::runtime_error("in sever block: unexpected line: " + line);
}

/**
 * @brief Parses the location block inside a server block
 */
void Config::parse_location(std::string & line, Location & block, std::stringstream & content)
{
	Log::log("inside parse location", DEBUG);
	block.set_path(line);
	int directive_count = 0;
	while (getline(content, line))
	{
		line = trim(line); // remove unnecessary whitespaces
		if (line.empty()) // skip empty lines
			continue ;

		if (line == "}" && directive_count == 0)
			throw std::runtime_error("empty location block");
		if (line == "}") // end of block
			return ;
		else if (line[line.size() - 1] == ';') // check if ';' is there and remove it from the line
		{
			line = line.substr(0, line.size() - 1);
			line = trim(line);
		} 
		else
			throw std::runtime_error("in location block: missing semicolon: " + line);
		parse_location_block_directives(line, block, content);
		directive_count++;
	}
}

/**
 * @brief Parses the directives inside a location block e.g. root, index
 */
void Config::parse_location_block_directives(std::string & line, Location & block, std::stringstream & content)
{
	std::string         directive, value;
	std::stringstream   ss(line);
	if (getline(ss, directive, ' ') && getline(ss, value))
	{
		if (directive == "root")
			block.set_root(value);
		else if (directive == "index")
			block.set_index(value);
		else if (directive == "upload")
			block.set_upload(value);
		else if (directive == "allowed_methods")
			block.set_allowed_methods(value);
		else if (directive == "autoindex")
			block.set_autoindex(value);
		else if (directive == "cgi_extension")
			block.set_cgi_extensions(value);
		else if (directive == "return")
			block.set_redirect(value);
		else
			throw std::runtime_error("in location block: unknown directive: " + directive);
	}
	else
		throw std::runtime_error("in location block: unexpected line: " + line);
}






