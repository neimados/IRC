/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdTopic.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guyar <guyar@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/25 16:28:06 by guyar            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"
#include "../../inc/Messages.hpp"

void sendToAll(std::string topic, std::string chan, std::vector<User> users) {
    if (chan.size() == 0)
        return;
    
    for (size_t i = 0; i < users.size(); i++)
    {
        // Send the message to the channel
        if (topic.size() == 0)
            users[i].sendToUser(RPL_NOTOPIC(users[i].getNickname(), chan));
        else
            users[i].sendToUser(RPL_TOPIC(users[i].getNickname(), chan, topic));
    }
}

void Server::cmdTopic(User *user, std::string cmd) {
	// Parse the command to get channel name and topic from inputs 'TOPIC #no' and 'TOPIC #no :er'
	std::string channel;
	std::string topic;
	std::string::size_type channel_start = cmd.find_first_of("#");
	std::string::size_type topic_start = cmd.find_first_of(":");
	
	if (channel_start == std::string::npos) {
		// Send the error message to the user
		sendToUser(user, "Error: missing channel name.");

		std::cout << RED << BOLD << "Missing channel name." << RESET << std::endl << std::endl;
		return;
	}
    
	channel = cmd.substr(channel_start, topic_start - channel_start - 1);
	
    size_t pos = channel.find("\r");
    if (pos != std::string::npos)
        channel.erase(pos);

    pos = channel.find("\n");
    if (pos != std::string::npos)
        channel.erase(pos);

    pos = channel.find(" ");

    if (pos != std::string::npos) {
        channel.erase(pos);
	}
	
	if (topic_start != std::string::npos) {
		topic = cmd.substr(topic_start + 1, cmd.size() - topic_start - 1);
	}
	
	int chan_index = findChan(channel);

    // Channel doesn't exists
    if (chan_index == -1) {
        user->sendToUser(ERR_NOSUCHCHANNEL(user->getNickname(), channel));
		std::cout << RED << BOLD << "Channel `" << channel << "` doesn't exist." << RESET << std::endl << std::endl;
        return;
	}

    // User isn't in the channel
    if (!user->isInChan(channel)) {
        user->sendToUser(ERR_NOTONCHANNEL(user->getNickname(), channel));
		std::cout << RED << BOLD << "User `" << user->getNickname() << "` is not in channel `" << channel << "`." << RESET << std::endl << std::endl;
		return;
	}

    // User is not operator and the mode +t is active
	if (
		_channels[chan_index].getIsTopicSettable() == true
		&& _channels[chan_index].isOperator(user) == false
	) {
        user->sendToUser(ERR_CHANOPRIVSNEEDED(user->getNickname(), _channels[chan_index].getName()));
		std::cout << RED << BOLD << "User `" << user->getNickname() << "` is not allowed to change the topic of `" << channel << "`." << RESET << std::endl << std::endl;
		return;
	}
	
	// Print the topic of the channel is it exists and is not empty
	if (topic.size() == 0) {
		if (_channels[chan_index].getTopic().size() != 0) {
			std::cout << GREEN << BOLD << "Topic of " << channel << ": " << _channels[chan_index].getTopic() << RESET << std::endl << std::endl;
            user->sendToUser(RPL_TOPIC(user->getNickname(), _channels[chan_index].getName(), _channels[chan_index].getTopic()));
		} else {
			std::cout << GREEN << BOLD << "Topic of " << channel << ": No topic set." << RESET << std::endl << std::endl;
            user->sendToUser(RPL_NOTOPIC(user->getNickname(), _channels[chan_index].getName()));
		}
		return;
	}

    // Update the topic
    _channels[chan_index].setTopic(topic);
    
    // Send the TOPIC message to the users
    sendToAll(topic, _channels[chan_index].getName(), _usrs);

    std::cout << GREEN << BOLD << user->getNickname() << " changed the topic of " << channel << " to " << topic << RESET << std::endl << std::endl;
}
