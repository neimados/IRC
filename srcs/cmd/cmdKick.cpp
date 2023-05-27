#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void Server::cmdKick(User *user, std::string cmd) {
	cmd.erase(0, 5); // Remove the "KICK " part of the command
	// Split the command into 3 parts
	std::string username;
	std::string channel;
	std::string reason = "No reason specified";
	size_t i = 0;

	// A for loop to get the username and channel name
	while (i < cmd.size()) {
		if (cmd[i] == ' ') {
			channel = cmd.substr(0, i);
			break;
		}
		i++;
	}

	cmd.erase(0, i + 1); // Remove the channel name from the command

	i = 0;
	// A for loop to get the username and channel name
	while (i < cmd.size()) {
		if (cmd[i] == ' ' || i == cmd.size() - 1) {
			username = clearString(cmd.substr(0, i + 1));
			break;
		}
		i++;
	}

	cmd.erase(0, i + 1); // Remove the username from the command

	// A for loop to get the reason
	if (cmd.size() > 0) {
		if (cmd[0] == ':') {
			cmd.erase(0, 1);
		}
		reason = cmd;
	}

	if (!user->isInChan(channel)) {
		// Send the error message to the user
		if (user->getisInChannel() == false)
			sendToUser(user, "Error: you are not in channel " + channel + ".");
		else
			sendToUserInChan(user, "404", channel, " you are not allowed to kick " + username + " from " + channel + ".");
		
		std::cout << RED << BOLD << "User " << user->getNickname() << " is not in channel " << channel << "." << RESET << std::endl << std::endl;
		return;
	}

	// Check if the channel exists with a loop
	int chan_index = findChan(channel);
	
	if (chan_index == -1) {
		// Send the error message to the user
		if (user->getisInChannel() == false)
			sendToUser(user, "Error: channel " + channel + " doesn't exist.");
		else
			sendToUserInChan(user, "404", channel, " channel " + channel + " doesn't exist.");

		std::cout << RED << BOLD << "Channel doesn't exist." << RESET << std::endl << std::endl;
		return;
	}

	//if user is not an operator
	if (_channels[chan_index].isOperator(user) == false){
		std::cout << RED << BOLD << "Only operators can kick user." << RESET << std::endl << std::endl;
		return;
	}


	// Check if the user exists with a loop
	for (size_t i = 0; i < _usrs.size(); i++) {
		if (_usrs[i].getNickname() == username) {
			// Send the KICK message to the user
			if (_usrs[i].getisInChannel() == false || !_usrs[i].isInChan(channel)) {
				sendToUser(user, "Error: user " + username + " is not in the channel " + channel + ".");
				std::cout << RED << BOLD << "User " << username << " is not in the channel " << channel << "." << RESET << std::endl << std::endl;
			} else {
				sendAllUsersInChan(channel, user->getNickname() + " KICK " + channel + " " + username + " :" + reason);
				sendToUser(&_usrs[i], user->getNickname() + " KICK " + channel + " " + username + " :" + reason);
	
				// Remove the user from the channel
				_channels[chan_index].delUsr(&_usrs[i]);

				// Remove privileges
				_channels[chan_index].delOperator(user, user);
				_channels[chan_index].delVoiced(user, user);
			
				_usrs[i].delChannel(_channels[chan_index].getName());
				
				// Send the NAMES message to the user
				cmdNames(user, channel);
				
				std::cout << GREEN << BOLD << user->getNickname() << " kicked " << username << " from " << channel << RESET << std::endl << std::endl;
			}
			return;
		}
	}

	// Send the error message to the user
	sendToUser(user, "Error: user " + username + " doesn't exist.");
	std::cout << RED << BOLD << "User " << username << " doesn't exist." << RESET << std::endl << std::endl;
}