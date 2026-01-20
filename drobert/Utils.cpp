/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:44:29 by drobert           #+#    #+#             */
/*   Updated: 2026/01/20 18:16:27 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <cctype>
#include <sstream>

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
        std::string l = it->second.getOutbuf() += data;
        it->second.setOutbuf(l);
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
		if (toUpper(it->second.getNick()) == toUpper(nick))
			return &it->second;
	}
	return NULL;
}

void Utils::sendFromClient(int to_fd, const Client& from, const std::string& cmd, const std::string& params, std::map<int, Client> &clients) {
	Utils::sendLine(to_fd, ":" + from.getPrefix() + " " + cmd + " " + params, clients);
}

bool Utils::iequals(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
		return false;
	
	for (size_t i = 0; i < a.size(); ++i)
	{
		if (std::tolower((unsigned char)a[i]) !=
			std::tolower((unsigned char)b[i]))
			return false;
	}
	return true;
}

std::string Utils::toString(int n)
{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}
