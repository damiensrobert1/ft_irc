/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 12:43:46 by drobert           #+#    #+#             */
/*   Updated: 2026/01/15 14:55:19 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client()
	: fd(-1), ip("")
{
}

Client::Client(int fd, std::string ip)
	:fd(fd), ip(ip)
{
	(void)fd;
}

std::string Client::prefix() const {
	std::string n = nick.empty() ? "*" : nick;
	std::string u = user.empty() ? "unknown" : user;
	return n + "!" + u + "@" + (ip.empty() ? "0.0.0.0" : ip);
}
