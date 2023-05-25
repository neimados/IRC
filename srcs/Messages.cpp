/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messages.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 08:54:15 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/25 11:29:14 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Messages.hpp"

// PASS USER
std::string RPL_WELCOME (std::string nickname) {
    return ("001 " + nickname + " :Hey " + nickname + " ! Welcome to our IRC :)");
}

std::string ERR_PASSWDMISMATCH (std::string nickname) {
    return ("464 " + nickname + " :Password incorrect");
}
std::string ERR_NOLOGIN (std::string nickname) {
    return ("465 " + nickname + " :You are not logged in");
}
std::string ERR_NEEDMOREPARAMS (std::string nickname) {
    return ("461 " + nickname + " :Not enough parameters");
}

std::string ERR_ALREADYREGISTRED (std::string nickname) {
    return ("462 " + nickname + " :You may not reregister");
}

std::string ERR_NOTREGISTERED (std::string nickname) {
    return ("451 " + nickname + " :You have not registered");
}


// NICK
std::string RPL_NICK (std::string oldnick, std::string newnick) {
    return (":" + oldnick + " NICK " + newnick);
}

std::string ERR_NONICKNAMEGIVEN (std::string nickname) {
    return ("431 " + nickname + " :No nickname given");
}
std::string ERR_ERRONEUSNICKNAME (std::string nickname) {
    return ("432 " + nickname + " :Erroneus nickname");
}
std::string ERR_NICKNAMEINUSE (std::string nickname) {
    return ("433 " + nickname + " :Nickname is already in use");
}


// TOPIC
std::string RPL_TOPIC (std::string nickname, std::string channel, std::string topic) {
    return ("332 " + nickname + " " + channel + " :" + topic);
}
std::string RPL_NOTOPIC(std::string nickname, std::string channel) {
    return "331 " + nickname + " " + channel + " :No topic is set";
}

std::string ERR_CHANOPRIVSNEEDED(std::string nickname, std::string channel) {
    return "482 " + nickname + " " + channel + " :You're not channel operator";
}


// NAMES
std::string RPL_NAMREPLY (std::string nickname, std::string channel, std::string users) {
    return ("353 " + nickname + " = " + channel + " :" + users);
}

std::string RPL_ENDOFNAMES (std::string nickname, std::string channel) {
    return ("366 " + nickname + " " + channel + " :End of /NAMES list");
}

// JOIN
std::string RPL_JOIN (std::string nickname, std::string channel) {
    return (":" + nickname + " JOIN " + channel);
}

std::string ERR_NOSUCHCHANNEL (std::string nickname, std::string channel) {
    return ("403 " + nickname + " " + channel + " :No such channel");
}
std::string ERR_TOOMANYCHANNELS (std::string nickname, std::string channel) {
    return ("405 " + nickname + " " + channel + " :You have joined too many channels");
}
std::string ERR_TOOMANYTARGETS (std::string nickname, std::string channel) {
    return ("407 " + nickname + " " + channel + " :Duplicate recipients. No message delivered");
}
std::string ERR_INVITEONLYCHAN (std::string nickname, std::string channel) {
    return ("473 " + nickname + " " + channel + " :Cannot join channel (+i)");
}


// PART
std::string RPL_PART (std::string nickname, std::string channel) {
    return (":" + nickname + " PART " + channel);
}
std::string RPL_PART (std::string nickname, std::string channel, std::string message) {
    return (":" + nickname + " PART " + channel + " :" + message);
}

std::string ERR_NOTONCHANNEL (std::string nickname, std::string channel) {
    return ("442 " + nickname + " " + channel + " :You're not on that channel");
}


// LIST
std::string RPL_LIST (std::string nickname, std::string channel, std::string nb, std::string topic) {
    return ("322 " + nickname + " " + channel + " " + nb + " :" + topic);
}
std::string RPL_LISTSTART (std::string nickname) {
    return ("321 " + nickname + " Channel :Users Name");
}
std::string RPL_LISTEND (std::string nickname) {
    return ("323 " + nickname + " :End of /LIST");
}


// QUIT
std::string RPL_QUIT (std::string nickname) {
    return (":" + nickname + " QUIT");
}
std::string RPL_QUIT (std::string nickname, std::string message) {
    return (":" + nickname + " QUIT :" + message);
}
