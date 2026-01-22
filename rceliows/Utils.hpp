/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:42:49 by drobert           #+#    #+#             */
/*   Updated: 2026/01/22 20:00:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include "Client.hpp"

class Utils {
public:
	static std::string trimCRLF(const std::string& s);
	static std::string toUpper(std::string s);
	static void sendLine(int fd, const std::string& line,
		std::map<int, Client>& clients);
	static void queueSend(int fd, const std::string& data,
		std::map<int, Client>& clients);
	static Client* findByNick(const std::string& nick,
		std::map<int, Client>& clients);
	static void sendFromClient(int to_fd, const Client& from,
		const std::string& cmd, const std::string& params,
		std::map<int, Client>& clients);
	static bool iequals(const std::string& a, const std::string& b);
	static std::string toString(int n);
};