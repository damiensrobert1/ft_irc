#include <sstream>

std::vector<std::string>	splitArg(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, delimiter))
    {
        if (!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}

void Cmd::join()
{
	Client &client = clients[fd];
	if (parsed.args.empty() && !parsed.hasTrailing) {
		sendNumeric(client.fd, "461", "JOIN :Not enough parameters");
		return;
	}
	std::string chanNameStr = parsed.hasTrailing ? parsed.trailing : parsed.args[0];
	std::vector<std::string> channelNames = splitArg(chanNameStr, ',');
	
	std::vector<std::string> providedKeys;
	if (parsed.args.size() >= 2)
		providedKeys = splitArg(parsed.args[1], ',');
	
	for (size_t i = 0; i < channelNames.size(); ++i)
    {
		std::string chanName = channelNames[i];
		std::string providedKey = "";
        if (i < providedKeys.size())
            providedKey = providedKeys[i];

		if (chanName.empty() || chanName[0] != '#') {
			sendNumeric(client.fd, "479", chanName + " :Illegal channel name");
			return;
		}
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
}

void Cmd::part()
{
	Client &client = clients[fd];
	if (parsed.args.empty()) {
		sendNumeric(client.fd, "461", "PART :Not enough parameters");
		return;
	}
	
	std::vector<std::string> channelNames = splitArg(parsed.args[0], ',');
	std::string reason = parsed.hasTrailing ? parsed.trailing : "Leaving";
	
	for (size_t i = 0; i < channelNames.size(); ++i)
    {
		std::string chanName = channelNames[i];
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
		
		std::string line = ":" + client.prefix() + " PART " + chanName + " :" + reason;
		
		for (std::set<int>::iterator mit = ch.members.begin();
			mit != ch.members.end(); ++mit)
		{
			Utils::sendLine(*mit, line, clients);
		}
		
		ch.members.erase(client.fd);
		ch.operators.erase(client.fd);
		ch.invited.erase(client.fd);
		
		if (ch.members.empty()) {
			channels.erase(chanName);
		}
	}
}