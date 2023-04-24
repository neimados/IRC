/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guyar <guyar@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 19:13:05 by guyar             #+#    #+#             */
/*   Updated: 2023/04/24 19:21:15 by guyar            ###   ########.fr       */
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
    void addUsr(User * user);
    std::string getName() const;

private:
    std::string _name;
    std::vector<User>		_usrs;

    // std::vector<string>      _Sended // buffer
    //ou
    // std::string              _Sended // buffer avec le msg qui a ete envoye;
    
    protected:
};