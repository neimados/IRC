/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:48:25 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/31 11:17:46 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"
#include "Channel.hpp"


class Server;
class Channel;

class User {

public:
    User();
    User(pollfd Client, int fd, int socket);
    ~User();

    void    sendToUser(std::string message);

    // Getters
    std::string getNickname() const;
    std::string getPassword() const;
    std::string getBuffer() const;

    int     getFd() const;
    int     getSocket() const;
    bool    getVerification();
    bool    getUserVerification();
    bool    getIsRegistered();
    bool    getPassOk();

    std::vector<std::string>    getAllChans();

    bool    isInChan(const std::string &name);
    
    void    addChannel(const std::string &name);
    void    delChannel(const std::string &name);
    void    setIsInChan(bool state);
    
    // Setters
    void    setNickname(std::string nickname);
    void    setPassword(std::string password);
    void    setUsername(std::string username);
    void    setIsRegistered(bool type);
    void    setVerification(bool type);
    void    setUserVerification(bool type);
    void    setPassOk(bool ok);
    void    setHostname(std::string hostname);
    void    setBuffer(std::string cmd);

private:
    pollfd      _client;
    int         _fd;
    int         _socket;
    std::string _username;
    std::string _nickname;
    std::string _password;
    std::string _hostname;
    std::string _cmdBuffer;

    bool        isRegistered;
    bool        isVerified;
    bool        userIsVerified;
    bool        isInChannel;
    bool        passOk;

    std::vector<std::string> _chans;
};
