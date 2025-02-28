/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 12:38:08 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 14:07:52 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Utils.hpp"
#include "Request.hpp"
#include "ServerBlock.hpp"
#include <stdexcept>

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

