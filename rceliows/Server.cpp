/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:57:12 by drobert           #+#    #+#             */
/*   Updated: 2026/01/22 20:00:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <csignal>
#include <cerrno>
#include "Server.hpp"
#include "Parsed.hpp"
#include "Command.hpp"
#include "Utils.hpp"

extern volatile sig_atomic_t g_running;

Server::Server(const std::string& port, const std::string& password)
	: port(port)
	, password(password)
	, listen_fd(-1)
	, pfds()
	, clients()
	, channels()
	, to_close() {
}

bool Server::start() {
	if (!createListenSocket())
		return false;
	std::cout << "Listening on port " << port << "...\n";
	return true;
}

bool Server::setNonBlocking(int fd) {
    return (fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
}

bool Server::createListenSocket() {
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo* res = NULL;
	int g = getaddrinfo(NULL, port.c_str(), &hints, &res);
	if (g != 0) {
		std::cerr << "getaddrinfo: " << gai_strerror(g) << "\n";
		return false;
	}
	int fd = -1;
	for (struct addrinfo* p = res; p; p = p->ai_next) {
		fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (fd < 0)
			continue;
		int yes = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (!setNonBlocking(fd)) {
			::close(fd);
			fd = -1;
			continue;
		}
		if (::bind(fd, p->ai_addr, p->ai_addrlen) == 0) {
			if (::listen(fd, SOMAXCONN) == 0) {
				listen_fd = fd;
				break;
			}
		}
		::close(fd);
		fd = -1;
	}
	freeaddrinfo(res);
	if (listen_fd < 0) {
		std::cerr << "Failed to bind/listen on port " << port << "\n";
		return false;
	}
	return true;
}

void Server::buildPollFds() {
	pfds.clear();
	pollfd p;
	p.fd = listen_fd;
	p.events = POLLIN;
	p.revents = 0;
	pfds.push_back(p);
	for (std::map<int, Client>::iterator it = clients.begin();
		it != clients.end(); ++it) {
		int fd = it->first;
		Client& c = it->second;
		pollfd pc;
		pc.fd = fd;
		pc.events = POLLIN;
		if (!c.getOutbuf().empty())
			pc.events |= POLLOUT;
		pc.revents = 0;
		pfds.push_back(pc);
	}
}

static std::string getClientIp(const sockaddr_storage& ss) {
	char buf[INET6_ADDRSTRLEN];
	if (ss.ss_family == AF_INET) {
		sockaddr_in* a = (sockaddr_in*)&ss;
		inet_ntop(AF_INET, &a->sin_addr, buf, sizeof(buf));
		return std::string(buf);
	} else if (ss.ss_family == AF_INET6) {
		sockaddr_in6* a = (sockaddr_in6*)&ss;
		inet_ntop(AF_INET6, &a->sin6_addr, buf, sizeof(buf));
		return std::string(buf);
	}
	return "unknown";
}

void Server::acceptClients() {
	while (true) {
		sockaddr_storage ss;
		socklen_t slen = sizeof(ss);
		int cfd = ::accept(listen_fd, (sockaddr*)&ss, &slen);
		if (cfd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			std::cerr << "accept failed" << std::endl;
			break;
		}
		if (!setNonBlocking(cfd)) {
			::close(cfd);
			continue;
		}
		std::string ip = getClientIp(ss);
		Client c(cfd, ip);
		clients[cfd] = c;
	}
}

void Server::markForClose(int fd) {
	to_close.insert(fd);
}

void Server::handleRead(int fd) {
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end())
		return;
	char buf[4096];
	while (true) {
		ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
		if (n > 0) {
			it->second.getInbuf().append(buf, (size_t)n);
			if (it->second.getInbuf().size() > 64 * 1024) {
				markForClose(fd);
				return;
			}
		} else if (n == 0) {
			markForClose(fd);
			return;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			markForClose(fd);
			return;
		}
	}
}

void Server::handleCommand(int fd, const std::string& line) {
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end())
		return;
	Client& c = it->second;
	Parsed p(line);
	p.parse();
	if (p.getCmd().empty())
		return;
	if (p.getCmd() == "QUIT") {
		markForClose(fd);
		return;
	}
	if (p.getCmd() == "CAP")
		return;
	Cmd cmd(c.getFd(), p, clients, password, to_close, channels);
	if (p.getCmd() == "PASS") {
		cmd.pass();
		cmd.tryRegister();
		return;
	}
	if (p.getCmd() == "NICK") {
		cmd.nick();
		cmd.tryRegister();
		return;
	}
	if (p.getCmd() == "USER") {
		cmd.user();
		cmd.tryRegister();
		return;
	}
	if (!c.isRegistered()) {
		Utils::sendLine(fd, ":ircserv 451 * :You have not registered.",
			clients);
		return;
	}
	if (p.getCmd() == "PING") {
		Utils::sendLine(fd, ":ircserv PONG ircserv :" +
			(p.getHasTrailing() ? p.getTrailing() : ""), clients);
		return;
	}
	if (p.getCmd() == "USERHOST") {
		cmd.userhost();
		return;
	}
	if (p.getCmd() == "JOIN") {
		cmd.join();
		return;
	}
	if (p.getCmd() == "PART") {
		cmd.part();
		return;
	}
	if (p.getCmd() == "WHO") {
		cmd.who();
		return;
	}
	if (p.getCmd() == "WHOIS") {
		cmd.whois();
		return;
	}
	if (p.getCmd() == "PRIVMSG") {
		cmd.privmsg();
		return;
	}
	if (p.getCmd() == "KICK") {
		cmd.kick();
		return;
	}
	if (p.getCmd() == "INVITE") {
		cmd.invite();
		return;
	}
	if (p.getCmd() == "TOPIC") {
		cmd.topic();
		return;
	}
	if (p.getCmd() == "MODE") {
		cmd.mode();
		return;
	}
	Utils::sendLine(fd, ":ircserv 421 " + c.getNick() + " " + p.getCmd() +
		" :Unknown command", clients);
}

void Server::processInputLines() {
	for (std::map<int, Client>::iterator it = clients.begin();
		it != clients.end(); ++it) {
		int fd = it->first;
		if (to_close.count(fd))
			continue;
		Client& c = it->second;
		while (true) {
			size_t pos = c.getInbuf().find('\n');
			if (pos == std::string::npos)
				break;
			std::string line = c.getInbuf().substr(0, pos + 1);
			c.getInbuf().erase(0, pos + 1);
			line = Utils::trimCRLF(line);
			if (line.empty())
				continue;
			handleCommand(fd, line);
			if (to_close.count(fd))
				break;
		}
	}
}

void Server::handleWrite(int fd) {
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end())
		return;
	Client& c = it->second;
	while (!c.getOutbuf().empty()) {
		ssize_t n = ::send(fd, c.getOutbuf().data(), c.getOutbuf().size(), 0);
		if (n > 0) {
			c.getOutbuf().erase(0, (size_t)n);
		} else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			break;
		} else {
			markForClose(fd);
			break;
		}
	}
}

void Server::flushCloses() {
	for (std::set<int>::iterator it = to_close.begin();
		it != to_close.end(); ++it) {
		int fd = *it;
		removeClient(fd);
	}
	to_close.clear();
}

void Server::broadcastToChannel(const Channel& ch, int except_fd,
	const std::string& line) {
	const std::set<int>& memberSet = ch.getMembers();
	for (std::set<int>::const_iterator it = memberSet.begin();
		it != memberSet.end(); ++it) {
		int mfd = *it;
		if (mfd == except_fd)
			continue;
		Utils::sendLine(mfd, line, clients);
	}
}

void Server::partAllChannels(int fd) {
	std::vector<std::string> empty;
	for (std::map<std::string, Channel>::iterator it = channels.begin();
		it != channels.end(); ++it) {
		Channel& ch = it->second;
		if (!ch.isMember(fd))
			continue;
		Client& c = clients[fd];
		std::string partLine = ":" + c.getPrefix() + " PART " + ch.getName()
			+ " :leaving";
		broadcastToChannel(ch, fd, partLine);
		ch.removeMember(fd);
		if (ch.getMemberCount() == 0)
			empty.push_back(ch.getName());
	}
	for (size_t i = 0; i < empty.size(); ++i)
		channels.erase(empty[i]);
}

void Server::removeClient(int fd) {
	std::map<int, Client>::iterator it = clients.find(fd);
	if (it == clients.end()) {
		::close(fd);
		return;
	}
	partAllChannels(fd);
	::close(fd);
	clients.erase(it);
}

void Server::shutdownAll() {
	std::vector<int> fds;
	fds.reserve(clients.size());
	for (std::map<int, Client>::iterator it = clients.begin();
		it != clients.end(); ++it) {
		fds.push_back(it->first);
	}
	for (std::vector<int>::iterator it = fds.begin();
		it != fds.end(); ++it) {
		int fd = *it;
		removeClient(fd);
	}
	if (listen_fd >= 0) {
		::close(listen_fd);
		listen_fd = -1;
	}
}

void Server::loop() {
	while (g_running) {
		buildPollFds();
		int ret = ::poll(pfds.data(), pfds.size(), 250);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			std::cerr << "poll failed" << std::endl;
			break;
		}
		if (!pfds.empty() && (pfds[0].revents & POLLIN))
			acceptClients();
		for (size_t i = 1; i < pfds.size(); ++i) {
			if (pfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
				markForClose(pfds[i].fd);
				continue;
			}
			if (pfds[i].revents & POLLIN) {
				handleRead(pfds[i].fd);
			}
		}
		processInputLines();
		for (size_t i = 1; i < pfds.size(); ++i) {
			if (to_close.count(pfds[i].fd))
				continue;
			if (pfds[i].revents & POLLOUT)
				handleWrite(pfds[i].fd);
		}
		flushCloses();
	}
	shutdownAll();
}