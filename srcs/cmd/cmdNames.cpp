/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdNames.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/25 11:01:45 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void Server::cmdNames(User* user, std::string cmd) {
    // Remove NAMES from cmd
    cmd.erase(0, 6);

    // Search for \r or \n and remove them
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Get the message : everything after the first space
    std::string message = "";
    message += cmd.substr(cmd.find_first_of(" ") + 1);

    // Erase the message from cmd
    if (cmd.find_first_of(" ") != std::string::npos)
        cmd.erase(cmd.find_first_of(" "));
    else
        message.erase();

    // Check if channel is empty
    if (cmd.empty()) {
        user->sendToUser(ERR_NEEDMOREPARAMS("NAMES"));
        std::cerr << RED << "Channel name is empty." << RESET << std::endl << std::endl;
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
        std::string channelName = *it;
        // Check if channel is valid
        if (channelName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#") != std::string::npos || channelName.empty() || channelName.size() > 20) {
            user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), channelName));
            std::cerr << RED << "Channel name `" << channelName << "` is not valid." << RESET << std::endl << std::endl;
            continue;
        }

        // Find the channel index
        int chanIndex = findChan(channelName);

        if (chanIndex == -1) {
            user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), channelName));
            std::cerr << RED << "Channel `" << channelName << "` doesn't exist." << RESET << std::endl << std::endl;
            continue;
        }

        // Check if user is in the channel
        if (!_channels[chanIndex].isInChannel(user) && _channels[chanIndex].getIsPrivate()) {
            user->sendToUser(ERR_NOTONCHANNEL(user->getNickname(), channelName));
            std::cerr << RED << "User `" << user->getNickname() << "` is not in channel `" << channelName << "`." << RESET << std::endl << std::endl;
            continue;
        }

        // Sending list of users
        user->sendToUser(RPL_NAMREPLY(user->getNickname(), _channels[chanIndex].getName(), _channels[chanIndex].getChanUsrs()));
        user->sendToUser(RPL_ENDOFNAMES(user->getNickname(), _channels[chanIndex].getName()));

        std::cout << CYAN << "List of `" << channelName << "` is `" << RPL_NAMREPLY(user->getNickname(), _channels[chanIndex].getName(), _channels[chanIndex].getChanUsrs()) << "`." << RESET << std::endl << std::endl;
    }

    // Clear channels vector and message
    channels.clear();
    message.clear();
}
