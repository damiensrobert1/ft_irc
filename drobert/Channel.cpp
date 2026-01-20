/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 14:48:39 by drobert           #+#    #+#             */
/*   Updated: 2026/01/18 19:16:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel() 
    : name(""), 
      topic(""), 
      key(""), 
      hasKey(false),
      hasLimit(false),
      userLimit(0),
      inviteOnly(false),
      topicOpOnly(false)
{}

Channel::Channel(const Channel& other)
{
    this->name = other.name;
    this->topic = other.topic;
    this->key = other.key;
    this->hasKey = other.hasKey;
    this->members = other.members;
    this->operators = other.operators;
    this->invited = other.invited;
    this->hasLimit = other.hasLimit;
    this->userLimit = other.userLimit;
    this->inviteOnly = other.inviteOnly;
    this->topicOpOnly = other.topicOpOnly;
}

Channel& Channel::operator=(const Channel& other)
{
    if (this != &other)
    {
        this->name = other.name;
        this->topic = other.topic;
        this->key = other.key;
        this->hasKey = other.hasKey;
        this->members = other.members;
        this->operators = other.operators;
        this->invited = other.invited;
        this->hasLimit = other.hasLimit;
        this->userLimit = other.userLimit;
        this->inviteOnly = other.inviteOnly;
        this->topicOpOnly = other.topicOpOnly;
    }
    return *this;
}

Channel::~Channel() {}

// Getters

std::string Channel::getName() const
{
	return this->name;
}

std::string Channel::getTopic() const
{
	return this->topic;
}

std::string Channel::getKey() const
{
	return this->key;
}

bool Channel::getHasKey() const
{
	return this->hasKey;
}

bool Channel::getHasLimit() const
{
	return this->hasLimit;
}

int Channel::getUserLimit() const
{
	return this->userLimit;
}

bool Channel::getInviteOnly() const
{
	return this->inviteOnly;
}

bool Channel::getTopicOpOnly() const
{
	return this->topicOpOnly;
}

const std::set<int>& Channel::getMembers() const
{
	return this->members;
}

size_t Channel::getMemberCount() const
{
	return this->members.size();
}

bool Channel::isMember(int fd) const
{
	return members.count(fd) != 0;
}

bool Channel::isOp(int fd) const
{
	return operators.count(fd) != 0;
}

bool Channel::isInvited(int fd) const
{
	return invited.count(fd) != 0;
}

// Setters

void Channel::setName(const std::string& name)
{
	this->name = name;
}

void Channel::setTopic(const std::string& topic)
{
	this->topic = topic;
}

void Channel::setKey(const std::string& key)
{
	this->key = key;
	this->hasKey = true;
}

void Channel::removeKey() 
{
	this->key.clear();
	this->hasKey = false;
}

void Channel::setUserLimit(int limit)
{
	this->userLimit = limit;
	this->hasLimit = true;
}

void Channel::removeUserLimit()
{
	this->userLimit = 0;
	this->hasLimit = false;
}

void Channel::setInviteOnly(bool value)
{
	this->inviteOnly = value;
}

void Channel::setTopicOpOnly(bool value)
{
	this->topicOpOnly = value;
}

void Channel::addMember(int fd)
{
	members.insert(fd);
}

void Channel::addOperator(int fd)
{
	members.insert(fd);
	operators.insert(fd);
}

void Channel::addInvited(int fd)
{
	invited.insert(fd);
}

void Channel::removeMember(int fd)
{
	members.erase(fd);
	operators.erase(fd);
	invited.erase(fd);
}

void Channel::removeOperator(int fd)
{
	operators.erase(fd);
}

void Channel::removeInvited(int fd)
{
	invited.erase(fd);
}