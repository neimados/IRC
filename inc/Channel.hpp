/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 19:13:05 by guyar             #+#    #+#             */
/*   Updated: 2023/05/09 16:36:22 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"
#include "User.hpp"

class User;

class Channel {

public:
    Channel(std::string name);
    ~Channel();
    
    void addUsr(User *user);
    bool delUsr(User *user);

    bool getExternalMessage() const;
    bool getIsModerated() const;
    bool getIsPrivate() const;
    bool getIsTopicSettable() const;
    
    std::string getName() const;
    std::string getTopic() const;
    std::string getChanUsrs() const;
    std::string getNbUsers() const;
    std::string getMode() const;
    std::string getModeUser(User *user) const;

    bool isOperator(User *user) const;
    bool isVoiced(User *user) const;
    bool isInChannel(User *user) const;
    
    User * getChanUsr(int i);

    void setTopic(std::string topic);
    void updateUser(User *user);

    void addOperator(User *user, User *sender);
    void addVoiced(User *user, User *sender);
    void delOperator(User *user, User *sender);
    void delVoiced(User *user, User *sender);

    void setExternalMessage(bool externalMessage);
    void setIsModerated(bool isModerated);
    void setIsPrivate(bool isPrivate);
    void setIsTopicSettable(bool isTopicSettable);

private:

    std::string _name;
    std::string _topic;
    std::vector<User>         _usrs;

    // Vector of int for the operators fd
    std::vector<int>          _ops;
    // Vector of int for the voiced fd
    std::vector<int>          _voiced;

    bool _externalMessage;
    bool _isModerated;
    bool _isPrivate;
    bool _isTopicSettable;
    
};