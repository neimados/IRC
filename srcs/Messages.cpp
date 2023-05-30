/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Messages.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 08:54:15 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/30 22:46:58 by dvergobb         ###   ########.fr       */
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
std::string ERR_UNKNOWNCOMMAND (std::string nickname) {
    return ("421 " + nickname + " :Unknown command");
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

// PRIVMSG
std::string RPL_PRIVMSG(std::string nickname, std::string channel, std::string message) {
    return (":" + nickname + " PRIVMSG " + channel + " :" + message);
}

std::string ERR_NORECIPIENT(std::string nickname, std::string message) {
    return ("411 " + nickname + " " + message + " :No recipient given");
}
std::string ERR_CANNOTSENDTOCHAN(std::string nickname, std::string channel) {
    return ("404 " + nickname + " " + channel + " :Cannot send to channel");
}
std::string ERR_WILDTOPLEVEL(std::string nickname, std::string message) {
    return ("414 " + nickname + " " + message + " :Wildcard in toplevel domain");
}
std::string ERR_NOSUCHNICK(std::string user_nick, std::string dest_nick) {
    return ("401 " + user_nick + " " + dest_nick + " :No such nick/channel");
}
std::string ERR_NOTEXTTOSEND(std::string nickname) {
    return ("412 " + nickname + " :No text to send");
}
std::string ERR_NOTOPLEVEL(std::string nickname, std::string message) {
    return ("413 " + nickname + " " + message + " :No toplevel domain specified");
}
std::string ERR_TOOMANYTARGETS (std::string nickname, std::string channel) {
    return ("407 " + nickname + " " + channel + " :Duplicate recipients. No message delivered");
}

// NOTICE
std::string RPL_NOTICE(std::string nickname, std::string channel, std::string message) {
    return (":" + nickname + " NOTICE " + channel + " :" + message);
}


// MODE
std::string RPL_CHANNELMODEIS(std::string nickname, std::string channel, std::string mode) {
    return ("324 " + nickname + " " + channel + " " + mode);
}
std::string RPL_UMODEIS(std::string nickname, std::string mode) {
    return ("221 " + nickname + " " + mode);
}

std::string ERR_UNKNOWNMODE(std::string nickname, std::string mode) {
    return ("472 " + nickname + " " + mode + " :is unknown mode char to me");
}

std::string ERR_USERSDONTMATCH(std::string nickname) {
    return ("502 " + nickname + " :Cant change mode for other users");
}


// KICK
std::string RPL_KICK(std::string nickname, std::string channel, std::string user, std::string message) {
    return (":" + nickname + " KICK " + channel + " " + user + " :" + message);
}

std::string ERR_USERNOTINCHANNEL(std::string nickname, std::string user, std::string channel) {
    return ("441 " + nickname + " " + user + " " + channel + " :They aren't on that channel");
}
