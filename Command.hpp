/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 12:35:21 by drobert           #+#    #+#             */
/*   Updated: 2026/01/16 18:30:43 by drobert          ###   ########.fr       */
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
		Client client;
		Parsed parsed;
		std::map<int, Client> &clients;
		std::string password;
		std::set<int> to_close;
		std::map<std::string, Channel> &channels;

		void sendNumeric(int fd, const std::string& cmdOrNum, const std::string& msg);
		void markForClose(int fd);
		bool nickInUse(const std::string& nick, int except_fd) const;

	public:
		Cmd(Client &c, const Parsed &p, std::map<int, Client> &clients, std::string password, std::set<int> &to_close, std::map<std::string, Channel> &channels);
		void tryRegister();
		void pass();
		void nick();
		void user();
};
