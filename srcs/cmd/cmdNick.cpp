/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdNick.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/23 11:44:00 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

// Function nickIsValide
bool nickIsValide(std::string nickname) {
    // Check if nickname is valide
    if (nickname.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos)
        return (false);
    return (true);
}

// Function nickIsInUse
bool nickIsInUse(std::string nickname, std::vector<User> users) {
    // Check if nickname is already in use
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->getNickname() == nickname)
            return (true);
    }
    return (false);
}

void Server::cmdNick(User *user, std::string cmd) {
	// Remove NICK from cmd
    cmd.erase(0, 5);

    // Search for \r or \n and remove it
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Check if nickname is empty
    if (cmd == "") {
        user->sendToUser(ERR_NONICKNAMEGIVEN(cmd));
        std::cerr << RED << "Nickname is empty." << RESET << std::endl << std::endl;
        return ;
    }

    // Check if nickname is valide
    if (!nickIsValide(cmd) || cmd.size() > 9) {
        user->sendToUser(ERR_ERRONEUSNICKNAME(cmd));
        std::cerr << RED << "Nickname `" << cmd << "` is not valide." << RESET << std::endl << std::endl;
        return ;
    }

    // Check if nickname is already in use
    if (nickIsInUse(cmd, this->_usrs)) {
        user->sendToUser(ERR_NICKNAMEINUSE(cmd));
        std::cerr << RED << "Nickname `" << cmd << "` is already in use." << RESET << std::endl << std::endl;
        return ;
    }

    // Get old nickname
    std::string oldNick = user->getNickname();
    
    // Set new nickname
    user->setNickname(cmd);
    user->setVerification(true);

    if (user->getUserVerification() == true && user->getIsRegistered() == false) {
        user->setIsRegistered(true);
        user->sendToUser(RPL_WELCOME(user->getNickname()));
        std::cout << GREEN << BOLD << "User " << user->getNickname() << " registred." << RESET << std::endl;
    }

    // Check if user is in a channel
    if (user->getWhatChannel() == "") {
        // Send only to user
        user->sendToUser(RPL_NICK(oldNick, user->getNickname()));
    }
    
    // Send NICK to all users in the channel
    for (std::vector<Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
        if (it->getName() != "" && it->getName() == user->getWhatChannel()) {
            it->sendToChannel(RPL_NICK(oldNick, user->getNickname()));
            it->updateUser(user);
        }
    }

    std::cout << ORANGE << ITALIC << oldNick << RESET << " is now " << CYAN << BOLD << cmd << RESET << std::endl << std::endl;
}
