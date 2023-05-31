/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdJoin.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/31 11:21:22 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void addUserInChan(User *user, Channel &chan) {
    user->setIsInChan(true);
    user->addChannel(chan.getName());

    // Send message to all users in channel
    chan.sendToChannel(RPL_JOIN(user->getNickname(), chan.getName()));
    
    // Send message to user
    if (chan.getTopic().size() == 0)
        user->sendToUser(RPL_NOTOPIC(user->getNickname(), chan.getName()));
    else
        user->sendToUser(RPL_TOPIC(user->getNickname(), chan.getName(), chan.getTopic()));

    // NAME REPLY to user
    user->sendToUser(RPL_NAMREPLY(user->getNickname(), chan.getName(), chan.getChanUsrs()));
    user->sendToUser(RPL_ENDOFNAMES(user->getNickname(), chan.getName()));
}

void Server::cmdJoin(User *user, std::string cmd) {
	// Remove JOIN from cmd
    cmd.erase(0, 5);

    // Search for \r or \n and remove it
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Check if channel is empty
    if (cmd == "") {
        user->sendToUser(ERR_NEEDMOREPARAMS("JOIN"));
        std::cerr << RED << "Channel name is empty." << RESET << std::endl << std::endl;
        return ;
    }

    // Erase everything after the first space
    pos = cmd.find(" ");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Split cmd into channels vector (separated by ',')
    std::vector<std::string> channels;
    std::string tmp;
    std::stringstream ss(cmd);
    while (std::getline(ss, tmp, ',')) {
        channels.push_back(tmp);
    }

    // For each channel
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
        // Check if channel is valide
        if (it->find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#") != std::string::npos || it->size() == 0 || it->size() > 20) {
            user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), *it));
            std::cerr << RED << "Channel name `" << *it << "` is not valide." << RESET << std::endl << std::endl;
            continue ;
        }

        // Check if user is already in channel
        if (user->isInChan(*it)) {
            user->sendToUser(ERR_ALREADYREGISTRED(user->getNickname()));
            std::cerr << RED << "User `" << user->getNickname() << "` is already in channel `" << *it << "`." << RESET << std::endl << std::endl;
            continue ;
        }

        // Check if channel exists
        int chan_index = findChan(*it);

        // If channel doesn't exist, create it
        if (chan_index == -1) {
            Channel chan(*it);

            // Update chan index if part deleted one
            chan_index = findChan(*it);

            // Add user to channel
            chan.addUsr(user);
            chan.addVoiced(user, user);
            chan.addOperator(user, user);

            addUserInChan(user, chan);
            this->_channels.push_back(chan);

            std::cout << GREEN << "User `" << user->getNickname() << "` created and joined channel `" << *it << "` as operator." << RESET << std::endl << std::endl;
        } else {
            // Update chan index if part deleted one
            chan_index = findChan(*it);
            
            // Add user to channel
            _channels[chan_index].addUsr(user);

            addUserInChan(user, _channels[chan_index]);
            
            std::cout << GREEN << "User `" << user->getNickname() << "` joined channel `" << *it << "`." << RESET << std::endl << std::endl;
        }
    }
}