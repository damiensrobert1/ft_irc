/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 12:35:21 by drobert           #+#    #+#             */
/*   Updated: 2026/01/16 14:04:23 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <set>
#include <string>

#include "Client.hpp"
#include "Parsed.hpp"

class Cmd
{
	private:
		Client client;
		Parsed parsed;
		std::map<int, Client> &clients;
		std::string password;
		std::set<int> to_close;

		void sendNumeric(int fd, const std::string& cmdOrNum, const std::string& msg);
		void markForClose(int fd);

	public:
		Cmd(Client &c, const Parsed &p, std::map<int, Client> &clients, std::string password, std::set<int> &to_close);
		void pass();
};
