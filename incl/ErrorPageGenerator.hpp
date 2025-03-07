/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageGenerator.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 20:16:29 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/07 21:58:47 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <fstream>
#include "Utils.hpp"
#include "StatusCodes.hpp"
#include "Location.hpp"	
#include "Request.hpp"
#include "Log.hpp"

namespace ErrorPageGenerator
{
	std::string createErrorPage(const Request& req, const std::vector<ServerBlock>& server_blocks);
};