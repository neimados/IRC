/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdUser.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/06/01 09:00:14 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

static std::vector<std::string> splitCommand(const std::string& command) {
    std::vector<std::string> words;
    std::istringstream iss(command);
    std::string word;
    
    while (iss >> word) {
        words.push_back(word);
    }
    
    return words;
}

void Server::cmdUser(User *user, std::string cmd) {
	// Remove USER from cmd
    cmd.erase(0, 5);

    // Search for \r or \n and remove it
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Check if user is already registred
    if (user->getIsRegistered() == true) {
        user->sendToUser(ERR_ALREADYREGISTRED(user->getNickname()));
        std::cerr << RED << "User is already registred." << RESET << std::endl << std::endl;
        return ;
    }
    
    // Count words
    std::vector<std::string> words = splitCommand(cmd);

    // Check if username, hostname, servername or realname is empty
    if (words.size() != 4) {
        user->sendToUser(ERR_NEEDMOREPARAMS("USER"));
        std::cerr << RED << "Username, hostname, servername or realname is empty." << RESET << std::endl << std::endl;
        return ;
    }

    // If realname starts with ':', remove it
    if (words[3][0] == ':')
        words[3].erase(0, 1);
    
    // Print infos
    std::cout << CYAN << ITALIC << "Username: " << words[0] << RESET << NORMAL << std::endl;
	std::cout << CYAN << ITALIC << "Hostname: " << words[1] << RESET << NORMAL << std::endl;
	std::cout << CYAN << ITALIC << "Servername: " << words[2] << RESET << NORMAL << std::endl;
	std::cout << CYAN << ITALIC << "Realname: " << words[3] << RESET << NORMAL << std::endl << std::endl;
	
    // Set infos
    user->setUsername(words[0]);
    user->setHostname(words[1]);

    // Set isRegistered to true
    user->setUserVerification(true);

    if (user->getVerification() == true && user->getIsRegistered() == false) {
        user->setIsRegistered(true);
        user->sendToUser(RPL_WELCOME(user->getNickname()));
        std::cout << GREEN << BOLD << "User " << user->getNickname() << " registred." << RESET << std::endl;
    }

    // Check if user is in a channel
    if (user->getAllChans().size() != 0) {
        for (std::vector<Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
            if (user->isInChan(it->getName())) {
                it->updateUser(user);
                break ;
            }
        }
    }
}
