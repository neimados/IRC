/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdPass.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/23 11:19:32 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void Server::cmdPass(User *user, std::string cmd) {
	// Erase PASS from cmd
    cmd.erase(0, 5);

    // Search for \r or \n and remove it
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Check if password is empty
    if (cmd == "") {
        user->sendToUser(ERR_NEEDMOREPARAMS("PASS"));
        std::cerr << RED << "Password is empty." << RESET << std::endl << std::endl;
        return ;
    }
	
	user->setPassword(cmd);
	
	if (cmd == this->getPassword()) {
		user->setPassOk(true);
		std::cout << GREEN << BOLD << "User " << user->getNickname() << " connected." << RESET << std::endl;
	} else {
        user->sendToUser(ERR_PASSWDMISMATCH(user->getNickname()));
		std::cout << RED << BOLD << "Wrong password entered." << RESET << std::endl;
	}
}
