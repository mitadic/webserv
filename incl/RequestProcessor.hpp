/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mitadic <mitadic@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 12:38:08 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/18 21:53:13 by mitadic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdexcept>
#include "Utils.hpp"
#include "Exceptions.hpp"
#include "ContentTypes.hpp"
#include "ServerBlock.hpp"
#include "Log.hpp"
#include <stdexcept>
#include <algorithm>

#define MAX_REDIRECTS 5

class Request;

class RequestProcessor
{
	public:
		// for request handling
		std::string handleMethod(const Request& req, const std::vector<ServerBlock>& server_blocks);

	private:
		std::string processGet(const Request& req, const Location* location);
		std::string processPost(const Request& req,  const Location* location);
		std::string processDelete(const Request& req, const Location* location);
	};

