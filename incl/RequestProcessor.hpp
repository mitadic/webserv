/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 12:38:08 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/04 17:23:30 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdexcept>
#include "Utils.hpp"
#include "Exceptions.hpp"
#include "ContentTypes.hpp"
#include "ServerBlock.hpp"
#include <stdexcept>
#include <algorithm>

class Request;

class RequestProcessor
{
	public:
		// for request handling
		std::string handleMethod(const Request& req, const std::vector<ServerBlock>& server_blocks);

	private:
		std::string processGet(const Request& req, const std::vector<ServerBlock>& server_blocks);
		std::string processPost(const Request& req, const std::vector<ServerBlock>& server_blocks);
		std::string processDelete(const Request& req, const std::vector<ServerBlock>& server_blocks);
	};

