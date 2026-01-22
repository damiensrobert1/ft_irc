/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 12:36:32 by drobert           #+#    #+#             */
/*   Updated: 2026/01/22 18:30:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <string>
#include <cstdlib>
#include <algorithm>

#include "Command.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Utils.hpp"

Cmd::Cmd(
    int fd,
    const Parsed& p,
    std::map<int, Client>& clients,
    std::string password,
    std::set<int>& to_close,
    std::map<std::string, Channel>& channels
)
    : fd(fd)
    , parsed(p)
    , clients(clients)
    , password(password)
    , to_close(to_close)
    , channels(channels)
{
}

void Cmd::sendNumeric(int fd, const std::string& cmdOrNum, const std::string& msg) {
	std::string nick = "*";
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it != clients.end() && !it->second.getNick().empty())
		nick = it->second.getNick();
	Utils::sendLine(fd, ":ircserv " + cmdOrNum + " " + nick + " " + msg, clients);
}

void Cmd::markForClose(int fd) {
	to_close.insert(fd);
}

void Cmd::pass() {
	Client &client = clients[fd];
	if (parsed.getArgs().empty() && !parsed.getHasTrailing()) {
		sendNumeric(client.getFd(), "461", "PASS not enough parameters");
		return;
	}
	if (client.isRegistered()) {
		sendNumeric(client.getFd(), "462", ":You may not reregister");
		return;
	}
	std::string pass = parsed.getHasTrailing() ? parsed.getTrailing() : parsed.getArgs()[0];
	if (pass == password) {
		client.addAuthed();
		sendNumeric(client.getFd(), "NOTICE", ":Password accepted.");
	}
	else
	{
		sendNumeric(client.getFd(), "464", ":Password incorrect");
		markForClose(client.getFd());
	}
	tryRegister();
}

bool Cmd::nickInUse(const std::string& nick, int except_fd) const {
	for (std::map<int, Client>::iterator it = clients.begin();
			it != clients.end(); ++it) {
		if (it->first == except_fd)
			continue;
		if (Utils::toUpper(it->second.getNick()) == Utils::toUpper(nick))
			return true;
	}
	return false;
}

void Cmd::nick() {
	Client &client = clients[fd];
	
	if (parsed.getArgs().empty() && !parsed.getHasTrailing()) {
		sendNumeric(client.getFd(), "431", ":No nickname given");
		return;
	}
	
	std::string newNick = parsed.getHasTrailing() ? parsed.getTrailing() : parsed.getArgs()[0];

	if (newNick.empty()) {
		sendNumeric(client.getFd(), "432", ":Erroneous nickname");
		return;
	}
	if (newNick.size() > 30)
		newNick.resize(30);
	if (nickInUse(newNick, client.getFd())) {
		sendNumeric(client.getFd(), "433", newNick + " :Nickname is already in use");
		return;
	}
	
	std::string oldNick = client.getNick();
	
	if (!oldNick.empty() && Utils::iequals(oldNick, newNick))
		return;
	
	client.setNick(newNick);

	if (oldNick.empty())
		return;
	
	std::string line = ":" + oldNick + "!" + client.getUser() + "@" + client.getIp() + " NICK :" + client.getNick();
	
	std::set<int> targets;
	targets.insert(client.getFd());
	
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
		Channel &ch = it->second;
		if (!ch.isMember(client.getFd()))
			continue;
		
		const std::set<int>& memberSet = ch.getMembers();
		for (std::set<int>::const_iterator mit = memberSet.begin(); mit != memberSet.end(); ++mit) {
			targets.insert(*mit);
		}
	}
	
	for (std::set<int>::iterator tit = targets.begin(); tit != targets.end(); ++tit) {
		Utils::sendLine(*tit, line, clients);
	}
}


void Cmd::user() {
	Client &client = clients[fd];

	if (parsed.getArgs().size() < 3 || (!parsed.getHasTrailing())) {
		sendNumeric(client.getFd(), "461", "USER :Not enough parameters");
		return;
	}
	if (!client.getUser().empty()) {
		sendNumeric(client.getFd(), "462", ":You may not reregister");
		return;
	}

	client.settUser(parsed.getArgs()[0]);
	client.setRealname(parsed.getTrailing());
}

void Cmd::userhost() {
	Client &client = clients[fd];

	if (parsed.getArgs().empty()) {
		sendNumeric(client.getFd(), "461", "USERHOST :Not enough parameters");
		return;
	}

	std::string reply;
	size_t limit = std::min<size_t>(5, parsed.getArgs().size());
	
	for (size_t i = 0; i < limit; ++i) {
		Client* t = Utils::findByNick(parsed.getArgs()[i], clients);
		if (!t)
			continue;
	
		std::string item = t->getNick() + "=+" + (t->getUser().empty() ? "unknown" : t->getUser()) + "@" + (t->getIp().empty() ? "0.0.0.0" : t->getIp());
	
		if (!reply.empty())
			reply += " ";
		reply += item;
	}
	
	sendNumeric(client.getFd(), "302", ":" + reply);
}

Channel &Cmd::getOrCreateChannel(const std::string& name) {
	std::map<std::string, Channel>::iterator it = channels.find(name);

	if (it != channels.end())
		return it->second;

	Channel ch;

	ch.setName(name);
	channels[name] = ch;
	return channels[name];
}

void Cmd::broadcastToChannel(const Channel& ch, int except_fd, const std::string& line) {
	const std::set<int>& memberSet = ch.getMembers();

	for (std::set<int>::const_iterator it = memberSet.begin();
			it != memberSet.end(); ++it) {
		int mfd = *it;
		if (mfd == except_fd)
			continue;
		Utils::sendLine(mfd, line, clients);
	}
}

void Cmd::sendNamesList(Client& c, Channel& ch) {
	std::string names;
	const std::set<int>& memberSet = ch.getMembers();

	for (std::set<int>::const_iterator it = memberSet.begin();
			it != memberSet.end(); ++it) {

		int mfd = *it;
		std::map<int, Client>::iterator iter = clients.find(mfd);

		if (iter == clients.end())
			continue;
		if (!names.empty())
			names += " ";
		if (ch.isOp(mfd))
			names += "@";
		names += iter->second.getNick().empty() ? "*" : iter->second.getNick();
	}
	sendNumeric(c.getFd(), "353", "= " + ch.getName() + " :" + names);
	sendNumeric(c.getFd(), "366", ch.getName() + " :End of /NAMES list.");
}

#include <sstream>

std::vector<std::string>	splitArg(const std::string &s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream iss(s);

	while (std::getline(iss, token, delimiter)) {
		if (!token.empty())
			tokens.push_back(token);
	}
	return tokens;
}

void Cmd::join() {
	Client &client = clients[fd];

	if (parsed.getArgs().empty() && !parsed.getHasTrailing()) {
		sendNumeric(client.getFd(), "461", "JOIN :Not enough parameters");
		return;
	}

	std::string chanNameStr = parsed.getHasTrailing() ? parsed.getTrailing() : parsed.getArgs()[0];

	if (chanNameStr.empty() || chanNameStr[0] != '#') {
		sendNumeric(client.getFd(), "479", chanNameStr + " :Illegal channel name");
		return;
	}

	std::vector<std::string> channelNames = splitArg(chanNameStr, ',');
	std::vector<std::string> providedKeys;

	if (parsed.getArgs().size() >= 2)
		providedKeys = splitArg(parsed.getArgs()[1], ',');
	
	for (size_t i = 0; i < channelNames.size(); ++i) {
		std::string chanName = channelNames[i];
		std::string providedKey = "";

		if (i < providedKeys.size())
			providedKey = providedKeys[i];

		if (chanName.empty() || chanName[0] != '#') {
			sendNumeric(client.getFd(), "479", chanName + " :Illegal channel name");
			return;
		}

		Channel& ch = getOrCreateChannel(chanName);

		if (ch.getHasLimit() && ch.getMemberCount() >= static_cast<size_t>(ch.getUserLimit()) && !ch.isMember(client.getFd())) {
			sendNumeric(client.getFd(), "471", ch.getName() + " :Cannot join channel (+l)");
			continue;
		}
		
		if (ch.getInviteOnly() && !ch.isMember(client.getFd())) {
			if (!ch.isInvited(client.getFd())) {
				sendNumeric(client.getFd(), "473", ch.getName() + " :Cannot join channel (+i)");
				continue;
			}
		}
		
		if (ch.getHasKey() && !ch.isMember(client.getFd())) {
			if (providedKey != ch.getKey()) {
				sendNumeric(client.getFd(), "475", ch.getName() + " :Cannot join channel (+k)");
				continue;
			}
		}
		
		if (ch.isMember(client.getFd()))
			continue;
		
		bool firstUser = (ch.getMemberCount() == 0);
		ch.addMember(client.getFd());
	
		if (firstUser)
			ch.addOperator(client.getFd()); 
		
		ch.removeInvited(client.getFd());
		
		std::string joinLine = ":" + client.getPrefix() + " JOIN :" + ch.getName();
		Utils::sendLine(client.getFd(), joinLine, clients);
		broadcastToChannel(ch, client.getFd(), joinLine);
		
		if (ch.getTopic().empty())
			sendNumeric(client.getFd(), "331", ch.getName() + " :No topic is set");
		else
			sendNumeric(client.getFd(), "332", ch.getName() + " :" + ch.getTopic());
		
		sendNamesList(client, ch);
	}
}

void Cmd::part() {
	Client &client = clients[fd];
	if (parsed.getArgs().empty()) {
		sendNumeric(client.getFd(), "461", "PART :Not enough parameters");
		return;
	}
	
	std::vector<std::string> channelNames = splitArg(parsed.getArgs()[0], ',');
	std::string reason = parsed.getHasTrailing() ? parsed.getTrailing() : "Leaving";
	
	for (size_t i = 0; i < channelNames.size(); ++i) {
		std::string chanName = channelNames[i];
		std::map<std::string, Channel>::iterator it = channels.find(chanName);
		if (it == channels.end()) {
			sendNumeric(client.getFd(), "403", chanName + " :No such channel");
			continue;
		}
		
		Channel& ch = it->second;
		
		if (!ch.isMember(client.getFd())) {
			sendNumeric(client.getFd(), "442", chanName + " :You're not on that channel");
			continue;
		}
		
		std::string line = ":" + client.getPrefix() + " PART " + chanName + " :" + reason;
		
		const std::set<int>& memberSet = ch.getMembers();
		for (std::set<int>::const_iterator mit = memberSet.begin(); mit != memberSet.end(); ++mit) {
			Utils::sendLine(*mit, line, clients);
		}
		
		ch.removeMember(client.getFd());
		
		if (ch.getMemberCount() == 0)
			channels.erase(chanName);
	}
}

void Cmd::who() {
	Client &client = clients[fd];
	std::string mask = parsed.getArgs().empty() ? "*" : parsed.getArgs()[0];
	sendNumeric(client.getFd(), "315", mask + " :End of /WHO list.");
}

void Cmd::whois() {
	Client &client = clients[fd];

	if (parsed.getArgs().empty()) {
		sendNumeric(client.getFd(), "461", "WHOIS :Not enough parameters");
		return;
	}
	
	std::string nick = parsed.getArgs()[0];
	Client* t = Utils::findByNick(nick, clients);
	
	if (!t) {
		sendNumeric(client.getFd(), "401", nick + " :No such nick");
		sendNumeric(client.getFd(), "318", nick + " :End of /WHOIS list.");
		return;
	}
	
	sendNumeric(client.getFd(), "311",
	    t->getNick() + " " +
	    t->getUser() + " " +
	    t->getIp() + " * :" +
	    t->getRealname()
	);
	
	sendNumeric(client.getFd(), "318", t->getNick() + " :End of /WHOIS list.");
}

void Cmd::privmsg() {
	Client &client = clients[fd];

	if (parsed.getArgs().empty()) {
		sendNumeric(client.getFd(), "411", ":No recipient given (PRIVMSG)");
		return;
	}
	if (!parsed.getHasTrailing()) {
		sendNumeric(client.getFd(), "412", ":No text to send");
		return;
	}

	std::string target = parsed.getArgs()[0];
	std::string text = parsed.getTrailing();
	
	if (!target.empty() && target[0] == '#') {
		std::map<std::string, Channel>::iterator it = channels.find(target);

		if (it == channels.end()) {
			sendNumeric(client.getFd(), "403", target + " :No such channel");
			return;
		}

		Channel& ch = it->second;

		if (!ch.isMember(client.getFd())) {
			sendNumeric(client.getFd(), "442", target + " :You're not on that channel");
			return;
		}
		
		std::string line = ":" + client.getPrefix() + " PRIVMSG " + target + " :" + text;
		broadcastToChannel(ch, client.getFd(), line);
	} else {
		Client* dst = Utils::findByNick(target, clients);

		if (!dst) {
			sendNumeric(client.getFd(), "401", target + " :No such nick");
			return;
		}

		Utils::sendFromClient(dst->getFd(), client, "PRIVMSG", dst->getNick() + " :" + text, clients);
	}
}

void Cmd::kick() {
	Client &client = clients[fd];

	if (parsed.getArgs().size() < 2) {
		sendNumeric(client.getFd(), "461", "KICK :Not enough parameters");
		return;
	}

	std::string chanName = parsed.getArgs()[0];
	std::string nick = parsed.getArgs()[1];
	std::string reason = parsed.getHasTrailing() ? parsed.getTrailing() : "Kicked";
	std::map<std::string, Channel>::iterator it = channels.find(chanName);

	if (it == channels.end()) {
		sendNumeric(client.getFd(), "403", chanName + " :No such channel");
		return;
	}

	Channel& ch = it->second;

	if (!ch.isMember(client.getFd())) {
		sendNumeric(client.getFd(), "442", chanName + " :You're not on that channel");
		return;
	}
	if (!ch.isOp(client.getFd())) {
		sendNumeric(client.getFd(), "482", chanName + " :You're not channel operator");
		return;
	}
	
	Client* victim = Utils::findByNick(nick, clients);

	if (!victim || !ch.isMember(victim->getFd())) {
		sendNumeric(client.getFd(), "441", nick + " " + chanName + " :They aren't on that channel");
		return;
	}
	
	std::string kickLine = ":" + client.getPrefix() + " KICK " + chanName + " " + victim->getNick() + " :" + reason;
	Utils::sendLine(victim->getFd(), kickLine, clients);
	broadcastToChannel(ch, victim->getFd(), kickLine);
	
	ch.removeMember(victim->getFd());
	
	if (ch.getMembers().empty())
		channels.erase(chanName);
}

void Cmd::invite() {
	Client &client = clients[fd];

	if (parsed.getArgs().size() < 2) {
		sendNumeric(client.getFd(), "461", "INVITE :Not enough parameters");
		return;
	}

	std::string nick = parsed.getArgs()[0];
	std::string chanName = parsed.getArgs()[1];
	std::map<std::string, Channel>::iterator it = channels.find(chanName);

	if (it == channels.end()) {
		sendNumeric(client.getFd(), "403", chanName + " :No such channel");
		return;
	}

	Channel& ch = it->second;
	
	if (!ch.isMember(client.getFd())) {
		sendNumeric(client.getFd(), "442", chanName + " :You're not on that channel");
		return;
	}
	if (!ch.isOp(client.getFd())) {
		sendNumeric(client.getFd(), "482", chanName + " :You're not channel operator");
		return;
	}
	
	Client* dst = Utils::findByNick(nick, clients);

	if (!dst) {
		sendNumeric(client.getFd(), "401", nick + " :No such nick");
		return;
	}
	if (ch.isMember(dst->getFd())) {
		sendNumeric(client.getFd(), "443", nick + " " + chanName + " :is already on channel");
		return;
	}
	
	ch.addInvited(dst->getFd());
	
	sendNumeric(client.getFd(), "341", nick + " " + chanName);
	Utils::sendFromClient(dst->getFd(), client, "INVITE", dst->getNick() + " :" + chanName, clients);
}

void Cmd::topic() {
	Client &client = clients[fd];

	if (parsed.getArgs().empty()) {
		sendNumeric(client.getFd(), "461", "TOPIC :Not enough parameters");
		return;
	}

	std::string chanName = parsed.getArgs()[0];
	std::map<std::string, Channel>::iterator it = channels.find(chanName);

	if (it == channels.end()) {
		sendNumeric(client.getFd(), "403", chanName + " :No such channel");
		return;
	}

	Channel& ch = it->second;
	
	if (!ch.isMember(client.getFd())) {
		sendNumeric(client.getFd(), "442", chanName + " :You're not on that channel");
		return;
	}
    if (!parsed.getHasTrailing()) {
        if (ch.getTopic().empty())
            sendNumeric(client.getFd(), "331", chanName + " :No topic is set");
        else
            sendNumeric(client.getFd(), "332", chanName + " :" + ch.getTopic());
        return;
    }
    if (ch.getTopicOpOnly() && !ch.isOp(client.getFd())) {
        sendNumeric(client.getFd(), "482", chanName + " :You're not channel operator");
        return;
    }

	ch.setTopic(parsed.getTrailing());
	std::string line = ":" + client.getPrefix() + " TOPIC " + chanName + " :" + ch.getTopic();
	Utils::sendLine(client.getFd(), line, clients);
	broadcastToChannel(ch, client.getFd(), line);
}

void Cmd::mode() {
	Client &client = clients[fd];

	if (parsed.getArgs().empty()) {
		sendNumeric(client.getFd(), "461", "MODE :Not enough parameters");
		return;
	}

	std::string target = parsed.getArgs()[0];
	
	if (target.empty() || target[0] != '#') {
		sendNumeric(client.getFd(), "472", ":Only channel MODE is supported here");
		return;
	}
	
	std::map<std::string, Channel>::iterator it = channels.find(target);
	if (it == channels.end()) {
		sendNumeric(client.getFd(), "403", target + " :No such channel");
		return;
	}
	Channel& ch = it->second;
	
	if (parsed.getArgs().size() == 1) {
		std::string modes = "+";
		if (ch.getInviteOnly())
			modes += "i";
		if (ch.getTopicOpOnly())
			modes += "t";
		if (ch.getHasKey())
			modes += "k";
		if (ch.getHasLimit())
			modes += "l";
		
		std::string params;
		if (ch.getHasKey()) params += " " + ch.getKey();
		if (ch.getHasLimit()) params += " " + Utils::toString(ch.getUserLimit());
		
		sendNumeric(client.getFd(), "324", ch.getName() + " " + modes + params);
		return;
	}
	
	if (!ch.isMember(client.getFd())) {
		sendNumeric(client.getFd(), "442", ch.getName() + " :You're not on that channel");
		return;
	}
	if (!ch.isOp(client.getFd())) {
		sendNumeric(client.getFd(), "482", ch.getName() + " :You're not channel operator");
		return;
	}
	
	std::string modeStr = parsed.getArgs()[1];
	bool adding = true;
	size_t argi = 2;
	
	std::string applied = "";
	std::vector<std::string> appliedParams;
	
	for (size_t i = 0; i < modeStr.size(); ++i) {
		char m = modeStr[i];
		if (m == '+') {
			adding = true;
			continue;
		}
		if (m == '-') {
			adding = false;
			continue;
		}
		
		if (m == 'i') {
			ch.setInviteOnly(adding);
			applied += (adding ? "+i" : "-i");
		} else if (m == 't') {
			ch.setTopicOpOnly(adding);
			applied += (adding ? "+t" : "-t");
		} else if (m == 'k') {
			if (adding) {
				if (argi >= parsed.getArgs().size()) {
					sendNumeric(client.getFd(), "461", "MODE :Not enough parameters");
					return;
				}
				ch.setKey(parsed.getArgs()[argi++]);
				applied += "+k";
				appliedParams.push_back(ch.getKey());
			} else {
				ch.removeKey();
				applied += "-k";
			}
		} else if (m == 'l') {
			if (adding) {
				if (argi >= parsed.getArgs().size()) {
					sendNumeric(client.getFd(), "461", "MODE :Not enough parameters");
					return;
				}
				ch.setUserLimit(std::max(0, ::atoi(parsed.getArgs()[argi++].c_str())));
				applied += "+l";
				appliedParams.push_back(Utils::toString(ch.getUserLimit()));
			} else {
				ch.removeUserLimit();
				applied += "-l";
			}
		} else if (m == 'o') {
			if (argi >= parsed.getArgs().size()) {
				sendNumeric(client.getFd(), "461", "MODE :Not enough parameters");
				return;
			}
			std::string nick = parsed.getArgs()[argi++];
			Client* dst = Utils::findByNick(nick, clients);
			if (!dst || !ch.isMember(dst->getFd())) {
				sendNumeric(client.getFd(), "441", nick + " " + ch.getName() + " :They aren't on that channel");
				return;
			}
			if (adding)
				ch.addOperator(dst->getFd());
			else
				ch.removeOperator(dst->getFd());
			
			applied += (adding ? "+o" : "-o");
			appliedParams.push_back(dst->getNick());
		} else {
			sendNumeric(client.getFd(), "472", std::string(1, m) + " :is unknown mode char to me");
			return;
		}
	}
	
	if (applied.empty())
		return;
	
	std::string params = ch.getName() + " " + applied;
	for (size_t i = 0; i < appliedParams.size(); ++i)
		params += " " + appliedParams[i];
	
	std::string line = ":" + client.getPrefix() + " MODE " + params;
	Utils::sendLine(client.getFd(), line, clients);
	broadcastToChannel(ch, client.getFd(), line);
}

void Cmd::tryRegister() {
	Client &client = clients[fd];
	if (client.isRegistered())
		return;
	if (!client.isAuthed())
		return;
	if (client.getNick().empty() || client.getUser().empty())
		return;

	client.addRegistered();

	sendNumeric(client.getFd(), "001", ":Welcome to ft_irc " + client.getPrefix());
	sendNumeric(client.getFd(), "002", ":Your host is ircserv");
	sendNumeric(client.getFd(), "003", ":This server is a minimal ft_irc implementation");
}