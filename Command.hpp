/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 12:35:21 by drobert           #+#    #+#             */
/*   Updated: 2026/01/19 04:32:15 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <set>
#include <string>

#include "Client.hpp"
#include "Parsed.hpp"
#include "Channel.hpp"

class Cmd
{
	private:
		int fd;
		Parsed parsed;
		std::map<int, Client> &clients;
		std::string password;
		std::set<int> to_close;
		std::map<std::string, Channel> &channels;

		void sendNumeric(int fd, const std::string& cmdOrNum, const std::string& msg);
		void markForClose(int fd);
		bool nickInUse(const std::string& nick, int except_fd) const;
		Channel &getOrCreateChannel(const std::string &name);
		void broadcastToChannel(const Channel& ch, int except_fd, const std::string& line);
		void sendNamesList(Client& c, Channel& ch);
	public:
		Cmd(int fd, const Parsed &p, std::map<int, Client> &clients, std::string password, std::set<int> &to_close, std::map<std::string, Channel> &channels);
		void tryRegister();
		void pass();
		void nick();
		void user();
		void join();
		void privmsg();
		void kick();
		void invite();
		void topic();
};
