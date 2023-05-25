/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdUser.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/25 10:08:40 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

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

    // Get <username> <hostname> <servername> :<realname>
    std::string username = cmd.substr(0, cmd.find(" "));
    cmd.erase(0, cmd.find(" ") + 1);
    
    std::string hostname = cmd.substr(0, cmd.find(" "));
    cmd.erase(0, cmd.find(" ") + 1);

    std::string servername = cmd.substr(0, cmd.find(" "));
    cmd.erase(0, cmd.find(" ") + 1);

    std::string realname = cmd;

    // If realname starts with ':', remove it
    if (realname[0] == ':')
        realname.erase(0, 1);

    // Check if username, hostname, servername or realname is empty
    if (username == "" || hostname == "" || servername == "" || realname == "") {
        user->sendToUser(ERR_NEEDMOREPARAMS("USER"));
        std::cerr << RED << "Username, hostname, servername or realname is empty." << RESET << std::endl << std::endl;
        return ;
    }
    
    // Print infos
    std::cout << CYAN << ITALIC << "Username: " << username << RESET << NORMAL << std::endl;
	std::cout << CYAN << ITALIC << "Hostname: " << hostname << RESET << NORMAL << std::endl;
	std::cout << CYAN << ITALIC << "Servername: " << servername << RESET << NORMAL << std::endl;
	std::cout << CYAN << ITALIC << "Realname: " << realname << RESET << NORMAL << std::endl << std::endl;
	
    // Set infos
    user->setUsername(username);
    user->setHostname(hostname);

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
            if (!user->isInChan(it->getName())) {
                it->updateUser(user);
                break ;
            }
        }
    }
}
