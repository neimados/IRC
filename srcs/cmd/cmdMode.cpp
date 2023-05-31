/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdMode.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/31 11:31:17 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

static void sendNewModes(std::string sender, std::string msg, std::vector<User> users) {
    if (msg.size() == 0 || sender.size() == 0)
        return;
    
    for (size_t i = 0; i < users.size(); i++) {
        users[i].sendToUser(msg);
    }
}

static void parseCommand(std::string command, std::string& channel, std::string& username, std::string& mode) {
    // Clear the variables
    channel.clear();
    username.clear();
    mode.clear();

    // Split the command into space-separated tokens
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    // Extract the relevant data based on the number of tokens
    if (tokens.size() == 1) {
        // Only username or channel is provided
		if (tokens[0][0] == '#')
			channel = tokens[0];
		else
        	username = tokens[0];
		return;
    }
	
	if (tokens.size() == 2) {
		if (tokens[0][0] == '#')
			channel = tokens[0];
		else
        	username = tokens[0];

		if (tokens[1][0] == '+' || tokens[1][0] == '-')
       		mode = tokens[1];
		else
			username = tokens[1];
		return;
    }
	
	if (tokens.size() == 3) {
        // Channel, username, and mode are provided
        channel = tokens[0];
        username = tokens[1];
        mode = tokens[2];
    }
}

void Server::cmdMode(User *user, std::string cmd) {
	// Remove the "MODE " part of the command
	cmd.erase(0, 5);

	// Parse the command as follow `#channel username +v` or `#channel +t`
	std::string channel;
	std::string username;
	std::string mode;

	parseCommand(cmd, channel, username, mode);

	// Copy the user to keep sender information
	User *sender = user;

	// Show channel modes or user in channel modes
	if (channel.size() > 0 && mode.size() == 0) {
		int chan_index = findChan(channel);

		// Check if the channel exists
		if (chan_index == -1) {
            user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), channel));
			std::cout << RED << BOLD << "Channel `" << channel << "` doesn't exist." << RESET << std::endl << std::endl;
			return;
		}

		// Check if the user is in the channel and if channel as externe message
		if (
			!user->isInChan(channel) &&
			_channels[chan_index].getExternalMessage() == false
		) {
			// Send the error message to the user
			user->sendToUser(ERR_USERNOTINCHANNEL(user->getNickname(), user->getNickname(), channel));
			std::cout << RED << BOLD << "User " << user->getNickname() << " is not in channel " << channel << "." << RESET << std::endl << std::endl;
			return;
		}


		// Print user's mode (any)
		if (username.size() > 0) {
			int user_index = findUser(username);

			if (user_index == -1) {
				// Send the error message to the user
				user->sendToUser(ERR_NOSUCHNICK(user->getNickname(), username));
				std::cout << RED << BOLD << "User `" << username << "` doesn't exist." << RESET << std::endl << std::endl;
				return;
			}
			
			// Send user mode on server : no particular mode
			user->sendToUser(RPL_UMODEIS(username, ""));
			std::cout << GREEN << BOLD << "User " << username << " has no mode." << RESET << std::endl << std::endl;
			return;
		}
			
		// Send the MODE message to the users
		user->sendToUser(RPL_CHANNELMODEIS(user->getNickname(), channel, _channels[chan_index].getMode()));
		std::cout << GREEN << BOLD << "Channel `" << channel << "` has mode `" << _channels[chan_index].getMode() << "`." << RESET << std::endl << std::endl;	
		return;
	}

	// Update channels modes or user in channel modes
	if (channel.size() > 0 && mode.size() > 0) {
		int chan_index = findChan(channel);

		// Check if the channel exists
		if (chan_index == -1) {
			user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), channel));
			std::cout << RED << BOLD << "Channel " << channel << " doesn't exist." << RESET << std::endl << std::endl;
			return;
		}
		
		// Check if the user is in the channel
		if (
			!user->isInChan(channel) &&
			_channels[chan_index].getExternalMessage() == false)
		{
			// Send the error message to the user
			user->sendToUser(ERR_USERNOTINCHANNEL(user->getNickname(), user->getNickname(), channel));
			std::cout << RED << BOLD << "User " << user->getNickname() << " is not in channel " << channel << "." << RESET << std::endl << std::endl;
			return;
		}

		// Check if the user is the channel's operator
		if (!_channels[chan_index].isOperator(user)) {
			user->sendToUser(ERR_CHANOPRIVSNEEDED(user->getNickname(), channel));
			std::cout << RED << BOLD << "User " << user->getNickname() << " is not the channel operator." << RESET << std::endl << std::endl;
			return;
		}
		
		// Check if the mode is valid
		if (mode.size() > 3 ||
			(mode[0] != '+' && mode[0] != '-') ||
			(mode[1] != 'o' && mode[1] != 'v' && mode[1] != 'm' && mode[1] != 'n' && mode[1] != 'p' && mode[1] != 't')
		) {
			user->sendToUser(ERR_UNKNOWNMODE(user->getNickname(), mode));
			std::cout << RED << BOLD << "Invalid mode." << RESET << std::endl << std::endl;
			return;
		}

		// Update user informations if username is not empty
		if (username.size() > 0) {
			int user_index = findUser(username);

			if (user_index == -1) {
				// Send the error message to the user
				user->sendToUser(ERR_NOSUCHNICK(user->getNickname(), username));
				std::cout << RED << BOLD << "User `" << username << "` doesn't exist." << RESET << std::endl << std::endl;
				return;
			}

			user = &_usrs[user_index];
		}
			
		// User's mode
		if (mode.substr(0, 2) == "-o") {
			_channels[chan_index].delOperator(user, sender);
			std::cout << GREEN << BOLD << user->getNickname() << " is not an operator. " << RESET << std::endl << std::endl;
			return;
		} else if (mode.substr(0, 2) == "+o") {
			_channels[chan_index].addOperator(user, sender);
			std::cout << GREEN << BOLD << user->getNickname() << " is an operator." << RESET << std::endl << std::endl;
			return;
		} else if (mode.substr(0, 2) == "-v") {
			_channels[chan_index].delVoiced(user, sender);
			std::cout << GREEN << BOLD << user->getNickname() << " is not voiced. " << RESET << std::endl << std::endl;
			return;
		} else if (mode.substr(0, 2) == "+v") {
			_channels[chan_index].addVoiced(user, sender);
			std::cout << GREEN << BOLD << user->getNickname() << " is voiced." << RESET << std::endl << std::endl;
			return;
		}

		// Channel's mode
		if (mode.substr(0, 2) == "-m") {
			_channels[chan_index].setIsModerated(false);
			std::cout << GREEN << BOLD << channel << " is not moderated." << RESET << std::endl << std::endl;
		} else if (mode.substr(0, 2) == "+m") {
			_channels[chan_index].setIsModerated(true);
			std::cout << GREEN << BOLD << channel << " is moderated." << RESET << std::endl << std::endl;
		} else if (mode.substr(0, 2) == "-n") {
			_channels[chan_index].setExternalMessage(true);
			std::cout << GREEN << BOLD << channel << " is not external message." << RESET << std::endl << std::endl;
		} else if (mode.substr(0, 2) == "+n") {
			_channels[chan_index].setExternalMessage(false);
			std::cout << GREEN << BOLD << channel << " is external message." << RESET << std::endl << std::endl;
		} else if (mode.substr(0, 2) == "-p") {
			_channels[chan_index].setIsPrivate(false);
			std::cout << GREEN << BOLD << channel << " is not private." << RESET << std::endl << std::endl;
		} else if (mode.substr(0, 2) == "+p") {
			_channels[chan_index].setIsPrivate(true);
			std::cout << GREEN << BOLD << channel << " is private." << RESET << std::endl << std::endl;
		} else if (mode.substr(0, 2) == "-t") {
			_channels[chan_index].setIsTopicSettable(false);
			std::cout << GREEN << BOLD << channel << " is not topic settable." << RESET << std::endl << std::endl;
		} else if (mode.substr(0, 2) == "+t") {
			_channels[chan_index].setIsTopicSettable(true);
			std::cout << GREEN << BOLD << channel << " is topic settable." << RESET << std::endl << std::endl;
		}
		
		sendNewModes(user->getNickname(), RPL_CHANNELMODEIS(user->getNickname(), channel, _channels[chan_index].getMode()), _channels[chan_index].getUsers());
		std::cout << GREEN << BOLD << "Mode changed." << RESET << std::endl << std::endl;
	}
}
