/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 16:57:17 by drobert           #+#    #+#             */
/*   Updated: 2026/01/18 18:39:18 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <set>

class Client
{
	public:
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

		Client();
		Client(int fd, std::string ip);

		std::string prefix() const;
};
