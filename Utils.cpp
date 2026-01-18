/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:44:29 by drobert           #+#    #+#             */
/*   Updated: 2026/01/18 20:33:29 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>

#include "Utils.hpp"

std::string Utils::trimCRLF(const std::string &s)
{
	size_t end = s.size();
	while (end > 0 && (s[end - 1] == '\r' || s[end - 1] == '\n'))
		end--;
	return s.substr(0, end);
}

std::string Utils::toUpper(std::string s)
{
	for (size_t i = 0; i < s.size(); ++i)
		s[i] = (char)std::toupper((unsigned char)s[i]);
	return s;
}

void Utils::queueSend(int fd, const std::string &data, std::map<int, Client> &clients)
{
        std::map<int, Client>::iterator it = clients.find(fd);
        if (it == clients.end())
                return;
        it->second.outbuf += data;
}

void Utils::sendLine(int fd, const std::string &line, std::map<int, Client> &clients)
{
        queueSend(fd, line + "\r\n", clients);
}

Client *Utils::findByNick(const std::string& nick, std::map<int, Client> &clients)
{
	for (std::map<int, Client>::iterator it = clients.begin();
		it != clients.end();
		++it)
	{
		if (toUpper(it->second.nick) == toUpper(nick))
			return &it->second;
	}
	return NULL;
}

void Utils::sendFromClient(int to_fd, const Client& from, const std::string& cmd, const std::string& params, std::map<int, Client> &clients) {
	Utils::sendLine(to_fd, ":" + from.prefix() + " " + cmd + " " + params, clients);
}
