/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messages.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 08:48:22 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/23 17:51:43 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

/* Infos messages */

// WELCOME
std::string RPL_WELCOME (std::string nickname);

// NICK
std::string RPL_NICK (std::string oldnick, std::string newnick);

// TOPIC
std::string RPL_TOPIC (std::string nickname, std::string channel, std::string topic);

// NAMES
std::string RPL_NAMREPLY (std::string nickname, std::string channel, std::string users);
std::string RPL_ENDOFNAMES (std::string nickname, std::string channel);

// JOIN
std::string RPL_JOIN (std::string nickname, std::string channel);

// PART
std::string RPL_PART (std::string nickname, std::string channel);
std::string RPL_PART (std::string nickname, std::string channel, std::string message);

// LIST
std::string RPL_LIST (std::string nickname, std::string channel, std::string nb, std::string topic);
std::string RPL_LISTSTART (std::string nickname);
std::string RPL_LISTEND (std::string nickname);

// QUIT
std::string RPL_QUIT (std::string nickname);
std::string RPL_QUIT (std::string nickname, std::string message);



/* Error messages */

// PASS
std::string ERR_PASSWDMISMATCH (std::string nickname);
std::string ERR_NOLOGIN (std::string nickname);

// NICK
std::string ERR_NONICKNAMEGIVEN (std::string nickname);
std::string ERR_ERRONEUSNICKNAME (std::string nickname);
std::string ERR_NICKNAMEINUSE (std::string nickname);

// USER, PASS
std::string ERR_NEEDMOREPARAMS (std::string nickname);
std::string ERR_ALREADYREGISTRED (std::string nickname);
std::string ERR_NOTREGISTERED (std::string nickname);

// JOIN
std::string ERR_NOSUCHCHANNEL (std::string nickname, std::string channel);
std::string ERR_TOOMANYCHANNELS (std::string nickname, std::string channel);
std::string ERR_TOOMANYTARGETS (std::string nickname, std::string channel);
std::string ERR_INVITEONLYCHAN (std::string nickname, std::string channel);

// PART
std::string ERR_NOTONCHANNEL (std::string nickname, std::string channel);
