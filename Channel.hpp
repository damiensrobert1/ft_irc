/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 16:57:42 by drobert           #+#    #+#             */
/*   Updated: 2026/01/18 19:15:42 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <set>
#include <string>

class Channel
{
	public:
		Channel();

		std::set<int> members;
		std::set<int> operators;
		std::set<int> invited;

		std::string name;
		std::string topic;

		bool hasLimit;
		int userLimit;

		bool inviteOnly;
		bool topicOpOnly;

		bool hasKey;
		std::string key;

		bool isMember(int fd) const;
		bool isOp(int fd) const;
};
