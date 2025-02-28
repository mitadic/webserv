/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageGenerator.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 20:16:29 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 20:31:10 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <fstream>
#include "Utils.hpp"


class ErrorPageGenerator
{
	public:
		static std::string createErrorPage(const Request& req, const std::vector<ServerBlock>& server_blocks);
};