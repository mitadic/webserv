/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 19:41:33 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/24 19:44:26 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

bool Utils::fileExists (const std::string& file)
{
	return (access(file.c_str(), F_OK) != -1);
}

// stat returns 0 on success, and -1 on failure (e.g., if the file does not exist)
bool Utils::isDirectory(const std::string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISDIR(info.st_mode);
}

std::string Utils::readFile(const std::string& file)
{
	std::ifstream read_file(file.c_str());
	if (!read_file.is_open())
	{
		throw std::runtime_error("Error opening the file");
	}
	std::stringstream buffer;
	buffer << read_file.rdbuf();
	read_file.close();
	return buffer.str();
}