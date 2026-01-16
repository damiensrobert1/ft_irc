/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:47:04 by drobert           #+#    #+#             */
/*   Updated: 2026/01/16 13:59:34 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <fcntl.h>
#include <poll.h>

#include <string>
#include <vector>
#include <map>
#include <set>

#include "Client.hpp"
#include "Channel.hpp"
#include "Parsed.hpp"

class Server
{
	private:
		std::string port;
		std::string password;
		int listen_fd;
		std::vector<pollfd> pfds;
		std::map<int, Client> clients;
		std::map<std::string, Channel> channels;
		std::set<int> to_close;

		bool setNonBlocking(int fd);
		bool createListenSocket();
		void buildPollFds();
		void acceptClients();
		void handleRead(int fd);
		void processInputLines();
		void handleCommand(int fd, const std::string& line);
		void handleWrite(int fd);
		void markForClose(int fd);
		void flushCloses();
		void partAllChannels(int fd);
		void removeClient(int fd);
		void broadcastToChannel(const Channel &ch, int except_fd, const std::string &line);
		void sendLine(int fd, const std::string& line);
		void queueSend(int fd, const std::string& data);
		void shutdownAll();
		void sendNumeric(int fd, const std::string& cmdOrNum, const std::string& msg);
		void cmdPASS(Client& c, const Parsed& p);
	public:

		Server(const std::string &port, const std::string &password);

		bool start();
		void loop();
};
