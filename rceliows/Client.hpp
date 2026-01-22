/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 16:57:17 by drobert           #+#    #+#             */
/*   Updated: 2026/01/22 20:00:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <set>

class Client {
private:
	int fd;
	std::string ip;
	std::string outbuf;
	std::string inbuf;
	std::string nick;
	std::string user;
	std::string realname;
	std::set<std::string> invited;
	bool registered;
	bool authed;

public:
	Client();
	Client(int fd, std::string ip);
	Client(const Client& other);
	Client& operator=(const Client& other);
	~Client();

	int getFd() const;
	std::string getIp() const;
	std::string getOutbuf() const;
	std::string getInbuf() const;
	std::string getNick() const;
	std::string getUser() const;
	std::string getRealname() const;
	std::string getPrefix() const;
	bool isRegistered() const;
	bool isAuthed() const;

	std::string& getOutbuf();
	std::string& getInbuf();

	void setFd(const int fd);
	void setIp(const std::string& ip);
	void setOutbuf(const std::string& outbuf);
	void setInbuf(const std::string& inbuf);
	void setNick(const std::string& nick);
	void settUser(const std::string& user);
	void setRealname(const std::string& realname);
	void addRegistered();
	void removeRegistered();
	void addAuthed();
	void removeAuthed();
};