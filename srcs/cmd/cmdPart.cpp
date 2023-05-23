/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdPart.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/23 18:53:14 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void Server::cmdPart(User* user, std::string cmd) {
    // Remove PART from cmd
    cmd.erase(0, 5);

    std::cout << "PART: " << cmd << std::endl;

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
        user->sendToUser(ERR_NEEDMOREPARAMS("PART"));
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
        if (!_channels[chanIndex].isInChannel(user)) {
            user->sendToUser(ERR_NOTONCHANNEL(user->getNickname(), channelName));
            std::cerr << RED << "User `" << user->getNickname() << "` is not in channel `" << channelName << "`." << RESET << std::endl << std::endl;
            continue;
        }

        // Remove user from the channel
        _channels[chanIndex].delUsr(user);
        user->setisInChannel(false);
        user->setWhatChannel("");
    
        if (!message.empty())
            _channels[chanIndex].sendToChannel(RPL_PART(user->getNickname(), channelName, message));
        else
            _channels[chanIndex].sendToChannel(RPL_PART(user->getNickname(), channelName));

        std::cout << CYAN << "User `" << user->getNickname() << "` left channel `" << channelName << "`." << RESET << std::endl << std::endl;

        // If the channel is empty, delete it
        if (_channels[chanIndex].getChanUsrs().empty()) {
            _channels.erase(_channels.begin() + chanIndex);
            std::cout << ORANGE << "Channel `" << channelName << "` deleted." << RESET << std::endl << std::endl;
        }
    }

    // Clear channels vector and message
    channels.clear();
    message.clear();
}
