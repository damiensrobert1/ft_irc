/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 12:36:32 by drobert           #+#    #+#             */
/*   Updated: 2026/01/18 15:34:11 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <string>

#include "Command.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Utils.hpp"

Cmd::Cmd(int fd, const Parsed& p, std::map<int, Client> &clients, std::string password, std::set<int> &to_close, std::map<std::string, Channel> &channels)
	:fd(fd), parsed(p), clients(clients), password(password), to_close(to_close), channels(channels)
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
	Client &client = clients[fd];
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
	}
	else
	{
		sendNumeric(client.fd, "464", ":Password incorrect");
		markForClose(client.fd);
	}
	tryRegister();
}

bool Cmd::nickInUse(const std::string& nick, int except_fd) const
{
	for (std::map<int, Client>::iterator it = clients.begin();
			it != clients.end();
			++it)
	{
		if (it->first == except_fd)
			continue;
		if (Utils::toUpper(it->second.nick) == Utils::toUpper(nick))
			return true;
	}
	return false;
}

#include <iostream>

void Cmd::nick()
{
	Client &client = clients[fd];
	if (parsed.args.empty() && !parsed.hasTrailing)
	{
		sendNumeric(client.fd, "431", ":No nickname given");
		return;
	}
	std::string newNick = parsed.hasTrailing ? parsed.trailing : parsed.args[0];
	if (newNick.empty())
	{
		sendNumeric(client.fd, "432", ":Erroneous nickname");
		return;
	}

	if (newNick.size() > 30)
		newNick.resize(30);
	
	if (nickInUse(newNick, client.fd))
	{
		sendNumeric(client.fd, "433", newNick + " :Nickname is already in use");
		return;
	}
	
	std::string oldNick = client.nick;
	client.nick = newNick;
	
	if (!oldNick.empty()) {
		for (std::map<std::string, Channel>::iterator it = channels.begin();
     				it != channels.end();
     				++it)
		{
			Channel& ch = it->second;
			if (ch.isMember(client.fd)) {
				for (std::set<int>::iterator it = ch.members.begin();
					it != ch.members.end();
					++it)
				{
					int mfd = *it;
					if (mfd == client.fd)
						continue;
					Utils::sendLine(mfd, ":" + oldNick + "!" + client.user + "@" + client.ip + " NICK :" + client.nick, clients);
				}
			}
		}
	}
}

void Cmd::user()
{
	Client &client = clients[fd];
	if (parsed.args.size() < 3 || (!parsed.hasTrailing)) {
		sendNumeric(client.fd, "461", "USER :Not enough parameters");
		return;
	}
	if (!client.user.empty()) {
		sendNumeric(client.fd, "462", ":You may not reregister");
		return;
	}
	client.user = parsed.args[0];
	client.realname = parsed.trailing;
}

void Cmd::userHost() {
	Client &client = clients[fd];
	if (parsed.args.empty())
	{
		sendNumeric(client.fd, "461", "USERHOST :Not enough parameters");
		return;
	}
	
	std::string reply;
	size_t limit = std::min<size_t>(5, parsed.args.size());
	
	for (size_t i = 0; i < limit; ++i) {
		Client* t = Utils::findByNick(parsed.args[i], clients);
		if (!t)
			continue;
		
		std::string item = t->nick + "=+" + (t->user.empty() ? "unknown" : t->user) + "@" + (t->ip.empty() ? "0.0.0.0" : t->ip);
		
		if (!reply.empty())
			reply += " ";
		reply += item;
	}
	
	sendNumeric(client.fd, "302", ":" + reply);
}

void Cmd::tryRegister()
{
	Client &client = clients[fd];
	if (client.registered)
		return;
	if (!client.authed)
		return;
	if (client.nick.empty() || client.user.empty())
		return;

	client.registered = true;

	sendNumeric(client.fd, "001", ":Welcome to ft_irc " + client.prefix());
	sendNumeric(client.fd, "002", ":Your host is ircserv");
	sendNumeric(client.fd, "003", ":This server is a minimal ft_irc implementation");
}
