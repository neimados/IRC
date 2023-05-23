/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:48:25 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/23 11:32:01 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"
#include "Channel.hpp"


class Server;

class User {

public:
    User();
    User(pollfd Client, int fd, int socket);
    ~User();

    void    sendToUser(std::string message);

    // Getters
    std::string getUsername() const;
    std::string getNickname() const;
    std::string getPassword() const;
    std::string getHostname() const;
    std::string getBuffer() const;

    int     getFd() const;
    int     getSocket() const;
    bool    getVerification();
    bool    getUserVerification();
    bool    getChannelVerification();
    bool    getIsRegistered();
    bool    getPassOk();

    pollfd  getPollFd();
    bool     getisInChannel();
    std::string getWhatChannel() const;
    
    // Setters
    void    setNickname(std::string nickname);
    void    setPassword(std::string password);
    void    setUsername(std::string username);
    void    setPort(int fd);
    void    setVerification(bool type);
    void    setUserVerification(bool type);
    void    setChannelVerification(bool type);
    void    setIsRegistered(bool type);
    void    setisInChannel(int n);
    void    setWhatChannel(std::string channelName);
    void    setPassOk(int ok);
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
    std::string _cmd;
    std::string _cmdBuffer;

    bool        isRegistered;
    bool        isVerified;
    bool        userIsVerified;
    bool        isInChannel;
    bool        passOk;

    std::string whatChannel;

    // User(const User &u);
    // User &operator=(const User &u);
};
