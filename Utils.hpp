/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:42:49 by drobert           #+#    #+#             */
/*   Updated: 2026/01/18 16:50:17 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>

#include "Client.hpp"

class Utils
{
	public:
		static std::string trimCRLF(const std::string &s);
		static std::string toUpper(std::string s);
		static void sendLine(int fd, const std::string &line, std::map<int, Client> &clients);
		static void queueSend(int fd, const std::string &data, std::map<int, Client> &clients);
		static Client *findByNick(const std::string& nick, std::map<int, Client> &clients);

};
