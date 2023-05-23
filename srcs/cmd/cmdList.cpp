/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdList.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/23 17:35:51 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void Server::cmdList(User *user, std::string cmd) {
	// Remove LIST from cmd
    cmd.erase(0, 5);

    // Search for \r or \n and remove them
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Check no channel name is given, print all
    if (cmd.empty()) {
        user->sendToUser(RPL_LISTSTART(user->getNickname()));
        for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
            std::string topic = it->getTopic();

            if (it->getIsPrivate())
                topic = "Private channel";
            else if (topic.empty())
                topic = "No topic is set";

            user->sendToUser(RPL_LIST(user->getNickname(), it->getName(), it->getNbUsers(), topic));
        }
        user->sendToUser(RPL_LISTEND(user->getNickname()));

        std::cout << CYAN << "User `" << user->getNickname() << "` listed all channels." << RESET << std::endl << std::endl;
        return;
    }

    // Split cmd into channels vector (separated by ',')
    std::vector<std::string> channels;
    std::string tmp;
    std::stringstream ss(cmd);
    while (std::getline(ss, tmp, ',')) {
        channels.push_back(tmp);
    }

    // For each channel
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
        // Check if channel is valid
        if (it->find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#") != std::string::npos || it->empty() || it->size() > 20) {
            user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), *it));
            std::cerr << RED << "Channel name `" << *it << "` is not valid." << RESET << std::endl << std::endl;
            continue;
        }

        // Find the channel index
        int chanIndex = findChan(*it);
        if (chanIndex == -1) {
            user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), *it));
            std::cerr << RED << "Channel `" << *it << "` does not exist." << RESET << std::endl << std::endl;
            continue;
        }

        // Send the channel info
        std::string topic = _channels[chanIndex].getTopic();

        if (_channels[chanIndex].getIsPrivate())
            topic = "Private channel";
        else if (topic.empty())
            topic = "No topic is set";

        user->sendToUser(RPL_LIST(user->getNickname(), _channels[chanIndex].getName(), _channels[chanIndex].getNbUsers(), topic));
    }

    std::cout << CYAN << "User `" << user->getNickname() << "` listed channels." << RESET << std::endl << std::endl;
}
