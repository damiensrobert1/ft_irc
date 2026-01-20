/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:41:13 by drobert           #+#    #+#             */
/*   Updated: 2026/01/12 16:30:51 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <iostream>

#include "Server.hpp"

volatile sig_atomic_t g_running = 1;
static void on_sigint(int)
{
	g_running = 0;
}

int main(int argc, char** argv)
{
	std::signal(SIGINT, on_sigint);
	std::signal(SIGTERM, on_sigint);
	
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
		return 1;
	}
	
	std::string port = argv[1];
	std::string password = argv[2];
	
	Server s(port, password);
	if (!s.start())
		return 1;
	s.loop();
	return 0;
}
