/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Messages.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 08:48:22 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/30 19:34:48 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

// PASS USER
std::string RPL_WELCOME (std::string nickname);

std::string ERR_PASSWDMISMATCH (std::string nickname);
std::string ERR_NOLOGIN (std::string nickname);
std::string ERR_NEEDMOREPARAMS (std::string nickname);
std::string ERR_ALREADYREGISTRED (std::string nickname);
std::string ERR_NOTREGISTERED (std::string nickname);


// NICK
std::string RPL_NICK (std::string oldnick, std::string newnick);

std::string ERR_NONICKNAMEGIVEN (std::string nickname);
std::string ERR_ERRONEUSNICKNAME (std::string nickname);
std::string ERR_NICKNAMEINUSE (std::string nickname);


// TOPIC
std::string RPL_TOPIC (std::string nickname, std::string channel, std::string topic);
std::string RPL_NOTOPIC(std::string nickname, std::string channel);

std::string ERR_CHANOPRIVSNEEDED(std::string nickname, std::string channel);


// NAMES
std::string RPL_NAMREPLY (std::string nickname, std::string channel, std::string users);
std::string RPL_ENDOFNAMES (std::string nickname, std::string channel);


// JOIN
std::string RPL_JOIN (std::string nickname, std::string channel);
// std::string RPL_JOIN (std::string nickname, std::string realname, std::string channel);

std::string ERR_NOSUCHCHANNEL (std::string nickname, std::string channel);
std::string ERR_TOOMANYCHANNELS (std::string nickname, std::string channel);
std::string ERR_INVITEONLYCHAN (std::string nickname, std::string channel);

// PART
std::string RPL_PART (std::string nickname, std::string channel);
std::string RPL_PART (std::string nickname, std::string channel, std::string message);

std::string ERR_NOTONCHANNEL (std::string nickname, std::string channel);


// LIST
std::string RPL_LIST (std::string nickname, std::string channel, std::string nb, std::string topic);
std::string RPL_LISTSTART (std::string nickname);
std::string RPL_LISTEND (std::string nickname);


// QUIT
std::string RPL_QUIT (std::string nickname);
std::string RPL_QUIT (std::string nickname, std::string message);

// PRIVMSG
std::string RPL_PRIVMSG(std::string nickname, std::string channel, std::string message);

std::string ERR_NORECIPIENT(std::string nickname, std::string message);
std::string ERR_CANNOTSENDTOCHAN(std::string nickname, std::string channel);
std::string ERR_WILDTOPLEVEL(std::string nickname, std::string message);
std::string ERR_NOSUCHNICK(std::string nickname, std::string message);
std::string ERR_NOTEXTTOSEND(std::string nickname);
std::string ERR_NOTOPLEVEL(std::string nickname, std::string message);
std::string ERR_TOOMANYTARGETS(std::string nickname, std::string message);

// NOTICE
std::string RPL_NOTICE(std::string nickname, std::string channel, std::string message);

// MODE
std::string RPL_CHANNELMODEIS(std::string nickname, std::string channel, std::string mode);

std::string ERR_USERNOTINCHANNEL(std::string nickname, std::string user, std::string channel);
std::string ERR_UNKNOWNMODE(std::string nickname, std::string mode);
std::string ERR_USERSDONTMATCH(std::string nickname);

// KICK
std::string RPL_KICK(std::string nickname, std::string channel, std::string user, std::string message);
