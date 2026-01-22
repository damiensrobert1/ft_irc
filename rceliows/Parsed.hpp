/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsed.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:22:50 by drobert           #+#    #+#             */
/*   Updated: 2026/01/22 18:30:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

class Parsed
{
	private:
		std::string line;
		std::string cmd;
		std::vector<std::string> args;
		std::string trailing;
		bool hasTrailing;

	public:
		Parsed(const std::string& line);
		Parsed(const Parsed& other);
		Parsed& operator=(const Parsed& other);
		~Parsed();

		void parse();

		// Getters
		const std::string& getCmd() const;
		const std::vector<std::string>& getArgs() const;
		const std::string& getTrailing() const;
		bool getHasTrailing() const;
};