/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 12:36:32 by drobert           #+#    #+#             */
/*   Updated: 2026/01/16 14:04:48 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <string>

#include "Command.hpp"
#include "Server.hpp"
#include "Utils.hpp"

Cmd::Cmd(Client& c, const Parsed& p, std::map<int, Client> &clients, std::string password, std::set<int> &to_close)
	:client(c), parsed(p), clients(clients), password(password), to_close(to_close)
{
}

void Cmd::sendNumeric(int fd, const std::string& cmdOrNum, const std::string& msg)
{
	std::string nick = "*";
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it != clients.end() && !it->second.nick.empty())
		nick = it->second.nick;
	Utils::sendLine(fd, ":ircserv " + cmdOrNum + " " + nick + " " + msg, clients);
}

void Cmd::markForClose(int fd)
{
        to_close.insert(fd);
}

void Cmd::pass()
{
	if (parsed.args.empty() && !parsed.hasTrailing) {
		sendNumeric(client.fd, "461", "PASS :Not enough parameters");
		return;
	}
	if (client.registered) {
		sendNumeric(client.fd, "462", ":You may not reregister");
		return;
	}
	std::string pass = parsed.hasTrailing ? parsed.trailing : parsed.args[0];
	if (pass == password) {
		client.authed = true;
		sendNumeric(client.fd, "NOTICE", ":Password accepted.");
	} else {
		sendNumeric(client.fd, "464", ":Password incorrect");
		markForClose(client.fd);
	}
}

