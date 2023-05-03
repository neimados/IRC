/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 19:13:05 by guyar             #+#    #+#             */
/*   Updated: 2023/05/03 10:55:51 by dvergobb         ###   ########.fr       */
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
    void delUsr(User *user);
    
    std::string getName() const;
    std::string getTopic() const;
    std::vector<std::string> getChanUsrs() const;
    
    User * getChanUsr(int i);

    void setTopic(std::string topic);

    // std::vector<User> &    get_usrs();

private:

    std::string _name;
    std::string _topic;
    std::vector<User>         _usrs;

    // std::vector<string>      _Sended // buffer
    //ou
    // std::string              _Sended // buffer avec le msg qui a ete envoye;
    
    protected:
};