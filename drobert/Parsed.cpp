/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:56:40 by drobert           #+#    #+#             */
/*   Updated: 2026/01/15 14:15:41 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <sstream>

#include "Parsed.hpp"
#include "Utils.hpp"

Parsed::Parsed(std::string line)
	: line(line), cmd(""), args(), trailing(""), hasTrailing(false)
{
}

void Parsed::parse()
{
	std::string s = line;
	if (!s.empty() && s[0] == ':')
	{
		size_t sp = s.find(' ');
		if (sp != std::string::npos) s = s.substr(sp + 1);
	}
	
	std::istringstream iss(s);
	iss >> cmd;
	cmd = Utils::toUpper(cmd);
	
	std::string token;
	while (iss >> token)
	{
		if (!token.empty() && token[0] == ':')
		{
			hasTrailing = true;
			trailing = token.substr(1);
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest[0] == ' ')
				 rest.erase(0, 1);
			if (!rest.empty())
			{
				if (!trailing.empty())
					trailing += " ";
				trailing += rest;
			}
		break;
		}
		else
		{
			args.push_back(token);
		}
	}
}
