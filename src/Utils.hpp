#pragma once

#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

namespace Utils
{
	bool fileExists (const std::string& file);
	bool isDirectory(const std::string& path);
	std::string readFile(const std::string& file);
}