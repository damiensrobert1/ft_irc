/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 12:36:32 by drobert           #+#    #+#             */
/*   Updated: 2026/01/19 03:35:39 by drobert          ###   ########.fr       */
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
		sendNumeric(client.fd, "461", "PASS not enough parameters");
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

void Cmd::nick()
{
	Client &client = clients[fd];
	
	if (parsed.args.empty() && !parsed.hasTrailing) {
		sendNumeric(client.fd, "431", ":No nickname given");
		return;
	}
	
	std::string newNick = parsed.hasTrailing ? parsed.trailing : parsed.args[0];
	if (newNick.empty()) {
		sendNumeric(client.fd, "432", ":Erroneous nickname");
		return;
	}
	if (newNick.size() > 30)
		newNick.resize(30);
	
	if (nickInUse(newNick, client.fd)) {
		sendNumeric(client.fd, "433", newNick + " :Nickname is already in use");
		return;
	}
	
	std::string oldNick = client.nick;
	
	if (!oldNick.empty() && Utils::iequals(oldNick, newNick))
		return;
	
	client.nick = newNick;
	
	if (oldNick.empty())
		return;
	
	std::string line = ":" + oldNick + "!" + client.user + "@" + client.ip + " NICK :" + client.nick;
	
	std::set<int> targets;
	targets.insert(client.fd);
	
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
		Channel &ch = it->second;
		if (!ch.isMember(client.fd))
			continue;
		
		for (std::set<int>::iterator mit = ch.members.begin(); mit != ch.members.end(); ++mit) {
			targets.insert(*mit);
		}
	}
	
	for (std::set<int>::iterator tit = targets.begin(); tit != targets.end(); ++tit) {
		Utils::sendLine(*tit, line, clients);
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

Channel &Cmd::getOrCreateChannel(const std::string& name)
{
	std::map<std::string, Channel>::iterator it = channels.find(name);
	if (it != channels.end())
		return it->second;
	Channel ch;
	ch.name = name;
	channels[name] = ch;
	return channels[name];
}

void Cmd::broadcastToChannel(const Channel& ch, int except_fd, const std::string& line) {
	for (std::set<int>::iterator it = ch.members.begin();
			it != ch.members.end();
			++it)
	{
		int mfd = *it;
		if (mfd == except_fd)
			continue;
		Utils::sendLine(mfd, line, clients);
	}
}

void Cmd::sendNamesList(Client& c, Channel& ch)
{
	std::string names;
	for (std::set<int>::iterator it = ch.members.begin();
			it != ch.members.end();
			++it)
	{
		int mfd = *it;
		std::map<int, Client>::iterator iter = clients.find(mfd);
		if (iter == clients.end())
			continue;
		if (!names.empty())
			names += " ";
		if (ch.isOp(mfd))
			names += "@";
		names += iter->second.nick.empty() ? "*" : iter->second.nick;
	}
	sendNumeric(c.fd, "353", "= " + ch.name + " :" + names);
	sendNumeric(c.fd, "366", ch.name + " :End of /NAMES list.");
}

void Cmd::join()
{
	Client &client = clients[fd];
	if (parsed.args.empty())
	if (parsed.args.empty() && !parsed.hasTrailing) {
		sendNumeric(client.fd, "461", "JOIN :Not enough parameters");
		return;
	}
	std::string chanName = parsed.hasTrailing ? parsed.trailing : parsed.args[0];
	if (chanName.empty() || chanName[0] != '#') {
		sendNumeric(client.fd, "479", chanName + " :Illegal channel name");
		return;
	}
	
	std::string providedKey;
	if (parsed.args.size() >= 2)
		providedKey = parsed.args[1];
	
	Channel& ch = getOrCreateChannel(chanName);
	
	if (ch.hasLimit && (int)ch.members.size() >= ch.userLimit && !ch.isMember(client.fd)) {
		sendNumeric(client.fd, "471", ch.name + " :Cannot join channel (+l)");
		return;
	}
	
	if (ch.inviteOnly && !ch.isMember(client.fd)) {
		if (!ch.invited.count(client.fd) && !client.invited.count(ch.name)) {
			sendNumeric(client.fd, "473", ch.name + " :Cannot join channel (+i)");
			return;
		}
	}
	
	if (ch.hasKey && !ch.isMember(client.fd)) {
		if (providedKey != ch.key) {
			sendNumeric(client.fd, "475", ch.name + " :Cannot join channel (+k)");
			return;
		}
	}
	
	if (ch.isMember(client.fd))
		return;
	
	bool firstUser = ch.members.empty();
	ch.members.insert(client.fd);
	
	if (firstUser)
		ch.operators.insert(client.fd);
	
	ch.invited.erase(client.fd);
	client.invited.erase(ch.name);
	
	std::string joinLine = ":" + client.prefix() + " JOIN :" + ch.name;
	Utils::sendLine(client.fd, joinLine, clients);
	broadcastToChannel(ch, client.fd, joinLine);
	
	if (!ch.topic.empty())
		sendNumeric(client.fd, "332", ch.name + " :" + ch.topic);
	else
		sendNumeric(client.fd, "331", ch.name + " :No topic is set");
	
	sendNamesList(client, ch);
}

void Cmd::privmsg()
{
	Client &client = clients[fd];
	if (parsed.args.empty()) {
		sendNumeric(client.fd, "411", ":No recipient given (PRIVMSG)");
		return;
	}
	if (!parsed.hasTrailing) {
		sendNumeric(client.fd, "412", ":No text to send");
		return;
	}
	
	std::string target = parsed.args[0];
	std::string text = parsed.trailing;
	
	if (!target.empty() && target[0] == '#') {
		std::map<std::string, Channel>::iterator it = channels.find(target);
		if (it == channels.end()) {
			sendNumeric(client.fd, "403", target + " :No such channel");
			return;
		}
		Channel& ch = it->second;
		if (!ch.isMember(client.fd)) {
			sendNumeric(client.fd, "442", target + " :You're not on that channel");
			return;
		}
		
		std::string line = ":" + client.prefix() + " PRIVMSG " + target + " :" + text;
		broadcastToChannel(ch, client.fd, line);
	} else {
		Client* dst = Utils::findByNick(target, clients);
		if (!dst) {
			sendNumeric(client.fd, "401", target + " :No such nick");
			return;
		}
		Utils::sendFromClient(dst->fd, client, "PRIVMSG", dst->nick + " :" + text, clients);
	}
}

void Cmd::kick()
{
	Client &client = clients[fd];
	if (parsed.args.size() < 2) {
		sendNumeric(client.fd, "461", "KICK :Not enough parameters");
		return;
	}
	std::string chanName = parsed.args[0];
	std::string nick = parsed.args[1];
	std::string reason = parsed.hasTrailing ? parsed.trailing : "Kicked";
	
	std::map<std::string, Channel>::iterator it = channels.find(chanName);
	if (it == channels.end()) {
		sendNumeric(client.fd, "403", chanName + " :No such channel");
		return;
	}
	Channel& ch = it->second;
	if (!ch.isMember(client.fd)) {
		sendNumeric(client.fd, "442", chanName + " :You're not on that channel");
		return;
	}
	if (!ch.isOp(client.fd)) {
		sendNumeric(client.fd, "482", chanName + " :You're not channel operator");
		return;
	}
	
	Client* victim = Utils::findByNick(nick, clients);
	if (!victim || !ch.isMember(victim->fd)) {
		sendNumeric(client.fd, "441", nick + " " + chanName + " :They aren't on that channel");
		return;
	}
	
	std::string kickLine = ":" + client.prefix() + " KICK " + chanName + " " + victim->nick + " :" + reason;
	Utils::sendLine(victim->fd, kickLine, clients);
	broadcastToChannel(ch, victim->fd, kickLine);
	
	ch.members.erase(victim->fd);
	ch.operators.erase(victim->fd);
	ch.invited.erase(victim->fd);
	
	if (ch.members.empty())
		channels.erase(chanName);
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
