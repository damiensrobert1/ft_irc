/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drobert <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 14:48:39 by drobert           #+#    #+#             */
/*   Updated: 2026/01/18 19:16:00 by drobert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel()
{
	inviteOnly = false;
	topicOpOnly = false;
	hasKey = false;
	hasLimit = false;
	userLimit = 0;
}

bool Channel::isMember(int fd) const
{
	return members.count(fd) != 0;
}

bool Channel::isOp(int fd) const
{
	return operators.count(fd) != 0;
}
