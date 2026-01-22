/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 12:43:46 by drobert           #+#    #+#             */
/*   Updated: 2026/01/22 20:00:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client()
	: fd(-1)
	, ip("")
	, outbuf("")
	, inbuf("")
	, nick("")
	, user("")
	, realname("")
	, invited()
	, registered(false)
	, authed(false) {
}

Client::Client(int fd, std::string ip)
	: fd(fd)
	, ip(ip)
	, outbuf("")
	, inbuf("")
	, nick("")
	, user("")
	, realname("")
	, invited()
	, registered(false)
	, authed(false) {
}

Client::Client(const Client& other)
	: fd(other.fd)
	, ip(other.ip)
	, outbuf(other.outbuf)
	, inbuf(other.inbuf)
	, nick(other.nick)
	, user(other.user)
	, realname(other.realname)
	, invited(other.invited)
	, registered(other.registered)
	, authed(other.authed) {
}

Client& Client::operator=(const Client& other) {
	if (this != &other) {
		this->fd = other.fd;
		this->ip = other.ip;
		this->outbuf = other.outbuf;
		this->inbuf = other.inbuf;
		this->nick = other.nick;
		this->user = other.user;
		this->realname = other.realname;
		this->invited = other.invited;
		this->registered = other.registered;
		this->authed = other.authed;
	}
	return *this;
}

Client::~Client() {
}

int Client::getFd() const {
	return this->fd;
}

std::string Client::getIp() const {
	return this->ip;
}

std::string Client::getOutbuf() const {
	return this->outbuf;
}

std::string Client::getInbuf() const {
	return this->inbuf;
}

std::string Client::getNick() const {
	return this->nick;
}

std::string Client::getUser() const {
	return this->user;
}

std::string Client::getRealname() const {
	return this->realname;
}

std::string Client::getPrefix() const {
	std::string n = nick.empty() ? "*" : nick;
	std::string u = user.empty() ? "unknown" : user;
	std::string i = ip.empty() ? "0.0.0.0" : ip;
	return n + "!" + u + "@" + i;
}

bool Client::isRegistered() const {
	return this->registered;
}

bool Client::isAuthed() const {
	return this->authed;
}

std::string& Client::getOutbuf() {
	return this->outbuf;
}

std::string& Client::getInbuf() {
	return this->inbuf;
}

void Client::setFd(const int fd) {
	this->fd = fd;
}

void Client::setIp(const std::string& ip) {
	this->ip = ip;
}

void Client::setOutbuf(const std::string& outbuf) {
	this->outbuf = outbuf;
}

void Client::setInbuf(const std::string& inbuf) {
	this->inbuf = inbuf;
}

void Client::setNick(const std::string& nick) {
	this->nick = nick;
}

void Client::settUser(const std::string& user) {
	this->user = user;
}

void Client::setRealname(const std::string& realname) {
	this->realname = realname;
}

void Client::addRegistered() {
	this->registered = true;
}

void Client::removeRegistered() {
	this->registered = false;
}

void Client::addAuthed() {
	this->authed = true;
}

void Client::removeAuthed() {
	this->authed = false;
}