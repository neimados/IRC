/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 19:13:05 by guyar             #+#    #+#             */
/*   Updated: 2023/05/07 11:58:50 by dvergobb         ###   ########.fr       */
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
    
    std::string getName() const;
    std::string getTopic() const;
    std::string getChanUsrs() const;
    
    User * getChanUsr(int i);

    void setTopic(std::string topic);
    void updateUser(User *user);

private:

    std::string _name;
    std::string _topic;
    std::vector<User>         _usrs;
    
    protected:
};