/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdPrivmsg.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guyar <guyar@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/29 16:27:34 by guyar            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void sendPrivmsg(std::string sender, std::string dest, std::string msg, std::vector<User> users) {
    if (msg.size() == 0 || dest.size() == 0 || sender.size() == 0)
        return;
    
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].getNickname() == sender)
            continue;
        users[i].sendToUser(RPL_PRIVMSG(sender, dest, msg));
    }
}

void Server::botSendMsg(std::string sender, std::string dest, std::string msg, std::vector<User> users) {
	
	int i;
	int range = 19 - 0 + 1;
	i = rand() % range + 0;
	
	(void)  msg; 
	std::stringstream buffer;
	std::string buff;

	std::ifstream ifs("text.txt");
	if (!ifs.good())
	{
		std::cout << "Infile error for text.txt" << std::endl;
		return;
	}

	buffer << ifs.rdbuf();
	buff = buffer.str();

	size_t pos;
	std::string delimiter = "\n";
	std::string token;
	std::vector<std::string> text;

	while ((pos = buff.find(delimiter)) != std::string::npos) {
	    token = buff.substr(0, pos);
	    text.push_back(token);
	    buff.erase(0, pos + delimiter.length());
	};

	sendPrivmsg(sender, dest, text[i], users);
}

void Server::cmdPrivmsg(User *user, std::string cmd) {
	std::string msg;
	std::string::size_type msg_start;
	
    // Remove PRIVMSG from the command
	cmd.erase(0, 8);

    // Search for \r or \n and remove them
    size_t pos = cmd.find("\r");
    if (pos != std::string::npos)
        cmd.erase(pos);

    pos = cmd.find("\n");
    if (pos != std::string::npos)
        cmd.erase(pos);
    
    // Check for message
	msg_start = cmd.find_first_of(":");
	if (msg_start == std::string::npos) {        
        // Sending ERR_NOTEXTTOSEND
        user->sendToUser(ERR_NOTEXTTOSEND(user->getNickname()));

		std::cout << RED << BOLD << "Invalid PRIVMSG message : no text to send." << RESET << std::endl << std::endl;
		return;
	}

    // Get the message
	msg = cmd.substr(msg_start + 1, cmd.size());
	
    // Check for destinataires
    if (cmd.find_first_of(" ") == std::string::npos){
        // Sending ERR_NORECIPIENT
        user->sendToUser(ERR_NORECIPIENT(user->getNickname(), "PRIVMSG"));

		std::cout << RED << BOLD << "Invalid PRIVMSG message." << RESET << std::endl << std::endl;
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
    std::cout << "MSG  :" << CYAN << BOLD << msg << RESET << std::endl << std::endl;
		
	size_t i = 0;

	//check duplicates
	for (size_t j = 0; j < destinataires.size(); j++) {
		for (size_t k = 0; k < destinataires.size(); k++) {
			if (destinataires[j] == destinataires[k] && j != k) {
				sendToUser(user, ERR_TOOMANYTARGETS(user->getNickname(), destinataires[i]));
				std::cout << RED << BOLD << "Duplicate recipients." << RESET << std::endl << std::endl;
				return;
			}
		}
	}

	// Checking all the strings in the vector destinataires if it's a channel or a user
	for (i = 0; i < destinataires.size(); i++) {
        if (destinataires[i][0] == '#') {
			// Destinataires[i] is a channel
			int chan_index = findChan(destinataires[i]);

            // Check if the channel exists
			if (chan_index == -1) {
				// Send the error message to the user
				sendToUser(user, ERR_CANNOTSENDTOCHAN(user->getNickname(), destinataires[i]));
				std::cout << RED << BOLD << "Channel " << destinataires[i] << " doesn't exist." << RESET << std::endl << std::endl;
				continue;
			}

            // Check if the channel is moderated and if the user is not an operator or a voiced user
			if (
				_channels[chan_index].getIsModerated() == true
				&& _channels[chan_index].isOperator(user) == false
				&& _channels[chan_index].isVoiced(user) == false
			) {
				// Send the error message to the user
                user->sendToUser(ERR_CANNOTSENDTOCHAN(user->getNickname(), destinataires[i]));
				std::cout << RED << BOLD << "Channel " << destinataires[i] << " is moderated." << RESET << std::endl << std::endl;
				continue;
			}

			// Check if the user is in the channel or if the user is an operator or a voiced user
			if (!_channels[chan_index].getExternalMessage() && !_channels[chan_index].isInChannel(user)) {
                user->sendToUser(ERR_CANNOTSENDTOCHAN(user->getNickname(), destinataires[i]));
				std::cout << RED << BOLD << "User " << user->getNickname() << " is not in the channel " << destinataires[i] << " without external messages." << RESET << std::endl << std::endl;
				continue;
			}
    
			if (msg == "!bot") {
				if(_channels[chan_index].getBotIsActivated() == 0) {
					_channels[chan_index].activateBot();
					std::cout << CYAN << BOLD << user->getNickname() << " activated the bot in chan " << destinataires[i] << RESET << std::endl << std::endl;
				}
				else {
					_channels[chan_index].desactivateBot();
					std::cout << CYAN << BOLD << user->getNickname() << " desactivated the bot in chan " << destinataires[i] << RESET << std::endl << std::endl;
				}
			}
		
		
			// Send the message to the channel
			sendPrivmsg(user->getNickname(), destinataires[i], msg, _channels[chan_index].getUsers());
			std::cout << GREEN << BOLD << user->getNickname() << " sent a message to " << destinataires[i] << RESET << std::endl << std::endl;
		

			if (_channels[chan_index].getBotIsActivated() == 1) {
				botSendMsg("BOT", destinataires[i], msg, _channels[chan_index].getUsers());
				std::cout << GREEN << BOLD << "BOT" << " sent a message to " << destinataires[i] << RESET << std::endl << std::endl;
			}
		}
		else {
			// Destinataires[i] is a username
			int user_index = findUser(destinataires[i]);

            // User doesn't exist
			if (user_index == -1) {
				sendToUser(user, ERR_NOSUCHNICK(user->getNickname(), destinataires[i]));
				std::cout << RED << BOLD << "User " << destinataires[i] << " doesn't exist." << RESET << std::endl << std::endl;
                continue;
			}
            
            // Send the message to the user
            _usrs[user_index].sendToUser(RPL_PRIVMSG(user->getNickname(), destinataires[i], msg));
            std::cout << GREEN << BOLD << user->getNickname() << " sent a message to " << destinataires[i] << RESET << std::endl << std::endl;
		}
    }
}
