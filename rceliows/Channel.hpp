/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 16:57:42 by drobert           #+#    #+#             */
/*   Updated: 2026/01/22 20:00:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <set>
#include <string>

class Channel {
private:
	std::string name;
	std::string topic;
	std::string key;
	bool hasKey;
	bool hasLimit;
	int userLimit;
	bool inviteOnly;
	bool topicOpOnly;
	std::set<int> members;
	std::set<int> operators;
	std::set<int> invited;

public:
	Channel();
	Channel(const Channel& other);
	Channel& operator=(const Channel& other);
	~Channel();

	std::string getName() const;
	std::string getTopic() const;
	std::string getKey() const;
	bool getHasKey() const;
	bool getHasLimit() const;
	int getUserLimit() const;
	bool getInviteOnly() const;
	bool getTopicOpOnly() const;
	const std::set<int>& getMembers() const;
	size_t getMemberCount() const;
	bool isMember(int fd) const;
	bool isOp(int fd) const;
	bool isInvited(int fd) const;

	void setName(const std::string& name);
	void setTopic(const std::string& topic);
	void setKey(const std::string& key);
	void removeKey();
	void setUserLimit(int limit);
	void removeUserLimit();
	void setInviteOnly(bool value);
	void setTopicOpOnly(bool value);
	void addMember(int fd);
	void addOperator(int fd);
	void addInvited(int fd);
	void removeMember(int fd);
	void removeOperator(int fd);
	void removeInvited(int fd);
};