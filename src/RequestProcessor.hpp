/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 12:38:08 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 12:53:02 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Utils.hpp"
#include "Request.hpp"
#include <stdexcept>

class RequestProcessor
{
	// for request handling
	std::string handleMethod(const Request& req);
	std::string processGet(const Request& req);
	std::string processPost(const Request& req);
	std::string processDelete(const Request& req);
};

