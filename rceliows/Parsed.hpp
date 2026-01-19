/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:22:50 by drobert           #+#    #+#             */
/*   Updated: 2026/01/15 14:10:21 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

class Parsed
{
	private:
		std::string line;
	public:
		std::string cmd;
		std::vector<std::string> args;
		std::string trailing;
		bool hasTrailing;

		Parsed(std::string line);

		void parse();
};
