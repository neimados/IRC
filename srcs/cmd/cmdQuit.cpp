/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdQuit.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/25 10:46:28 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void sendToAll(std::string message, std::vector<User> users) {
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->getIsRegistered())
            it->sendToUser(message + "\r\n");
    }
}

void Server::cmdQuit(User* user, std::string cmd, int fd) {
    // Remove QUIT from cmd
    cmd.erase(0, 5);

    // Search for \r or \n and remove them
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Get the message : everything after the first space
    std::string message = "";
    message += cmd;

    // Erase ":" from message
    if (message[0] == ':')
        message.erase(0, 1);

    // Send message to all users
    if (message.empty())
        sendToAll(RPL_QUIT(user->getNickname()), _usrs);
    else
        sendToAll(RPL_QUIT(user->getNickname(), message), _usrs);

    // Disconnect user
    disconnectUser(user, fd);
    
    std::cout << GREEN << "User " << user->getNickname() << " has quit with message `" << message << "`" << RESET << std::endl << std::endl;
}
