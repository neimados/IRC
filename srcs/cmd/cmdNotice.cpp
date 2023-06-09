/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdNotice.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/30 20:36:33 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

static void sendNotice(std::string sender, std::string dest, std::string msg, std::vector<User> users) {
    if (msg.size() == 0 || dest.size() == 0 || sender.size() == 0)
        return;
    
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].getNickname() == sender)
            continue;
        users[i].sendToUser(RPL_NOTICE(sender, dest, msg));
    }
}

void Server::cmdNotice(User *user, std::string cmd) {
	std::string msg;
	std::string::size_type msg_start;
	
    // Remove NOTICE from the command
	cmd.erase(0, 7);

    // Search for \r or \n and remove them
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);

    // Check if empty
    if (cmd.empty()) {
        std::cout << RED << BOLD << "Invalid NOTICE message : empty." << RESET << std::endl << std::endl;
        return;
    }
    
    // Check for message
	msg_start = cmd.find_first_of(":");
	if (msg_start == std::string::npos) {
		std::cout << RED << BOLD << "Invalid NOTICE message : no text to send." << RESET << std::endl << std::endl;
		return;
	}

    // Get the message
	msg = cmd.substr(msg_start + 1, cmd.size());
	
    // Check for destinataires
    if (cmd.find_first_of(" ") == std::string::npos){
        std::cout << RED << BOLD << "Invalid NOTICE message." << RESET << std::endl << std::endl;
		return;
	}

	// Looking for the `space` before the `:`, cutting to have just the list of destinaters separeted with comma
	cmd.erase(cmd.find_first_of(" "), cmd.size());;
    
	pos = 0;
	std::string delimiter = ",";
	std::string token;
	std::vector<std::string> destinataires;
    
	// Split cmd (there is only the list of destinataires at this point) string with the delimiter ","
	while ((pos = cmd.find(delimiter)) != std::string::npos) {
	    token = cmd.substr(0, pos);
	    destinataires.push_back(token);
	    cmd.erase(0, pos + delimiter.length());
	};
    
	destinataires.push_back(cmd);
	std::cout << "FROM :" << CYAN << BOLD << user->getNickname() << RESET << std::endl;
    std::cout << "TO   :";
	
    for (size_t i = 0; i < destinataires.size(); i++)
        std::cout << CYAN << BOLD << destinataires[i] << RESET << " ";

    std::cout << std::endl;
    std::cout << "MSG  :" << CYAN << BOLD << msg << RESET << std::endl;
		
	size_t i = 0;

	// Checking all the strings in the vector destinataires if it's a channel or a user
	for (i = 0; i < destinataires.size(); i++) {
        if (destinataires[i][0] == '#') {
			// Destinataires[i] is a channel
			int chan_index = findChan(destinataires[i]);

            // Check if the channel exists
			if (chan_index == -1) {
				std::cout << RED << BOLD << "Channel " << destinataires[i] << " doesn't exist." << RESET << std::endl << std::endl;
				continue;
			}

            // Check if the channel is moderated and if the user is not an operator or a voiced user
			if (
				_channels[chan_index].getIsModerated() == true
				&& _channels[chan_index].isOperator(user) == false
				&& _channels[chan_index].isVoiced(user) == false
			) {
				std::cout << RED << BOLD << "Channel " << destinataires[i] << " is moderated." << RESET << std::endl << std::endl;
				continue;
			}

			// Check if the user is in the channel or if the user is an operator or a voiced user
			if (!_channels[chan_index].getExternalMessage() && !_channels[chan_index].isInChannel(user)) {
                std::cout << RED << BOLD << "User " << user->getNickname() << " is not in the channel " << destinataires[i] << " without external messages." << RESET << std::endl << std::endl;
				continue;
			}
    
			// Send the message to the channel
			sendNotice(user->getNickname(), destinataires[i], msg, _channels[chan_index].getUsers());
			std::cout << GREEN << BOLD << user->getNickname() << " sent a notice `" << msg << "` to " << destinataires[i] << RESET << std::endl << std::endl;
		}
		else {
			// Destinataires[i] is a username
			int user_index = findUser(destinataires[i]);

            // User doesn't exist
			if (user_index == -1) {
				std::cout << RED << BOLD << "User " << destinataires[i] << " doesn't exist." << RESET << std::endl << std::endl;
                continue;
			}
            
            // Send the message to the user
            _usrs[user_index].sendToUser(RPL_NOTICE(user->getNickname(), destinataires[i], msg));
            std::cout << GREEN << BOLD << user->getNickname() << " sent a notice `" << msg << "` to " << destinataires[i] << RESET << std::endl << std::endl;
		}
    }
}
