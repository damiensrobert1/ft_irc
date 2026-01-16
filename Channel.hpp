/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 16:57:42 by drobert           #+#    #+#             */
/*   Updated: 2026/01/15 15:46:36 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <set>
#include <string>

class Channel
{
	public:
		std::set<int> members;
		std::set<int> operators;
		std::set<int> invited;

		std::string name;

		bool isMember(int fd) const;
};
