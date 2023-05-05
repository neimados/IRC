/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:48:25 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/02 11:37:32 by dvergobb         ###   ########.fr       */
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
    
    User    USER(std::string username);
    User    NICK(std::string nickname);

    // Getters
    std::string getUsername() const;
    std::string getNickname() const;
    std::string getPassword() const;
    std::string getHostname() const;

    int     getFd() const;
    int     getSocket() const;
    bool    getVerification();
    bool    getUserVerification();
    bool    getChannelVerification();
    bool    getPassOk();

    pollfd  getPollFd();
    int     getisInChannel();
    std::string getWhatChannel() const;
    
    // Setters
    void    setNickname(std::string nickname);
    void    setPassword(std::string password);
    void    setUsername(std::string username);
    void    setPort(int fd);
    void    setVerification(bool type);
    void    setUserVerification(bool type);
    void    setChannelVerification(bool type);
    void    setisInChannel(int n);
    void    setWhatChannel(std::string channelName);
    void    setPassOk(int ok);
    void    setHostname(std::string hostname);
    
    // Commands
    bool    channelLimit();
    void    increaseChannelNo();
    void    decreaseChannelNo();

private:
    pollfd      _client;
    int         _fd;
    int         _socket;
    std::string _username;
    std::string _nickname;
    std::string _password;
    std::string _hostname;
    std::string _cmd;

    bool        isVerified;
    bool        userIsVerified;
    bool        isInChannel;
    bool        passOk;

    std::string whatChannel;
    int noChannels;

    // User(const User &u);
    // User &operator=(const User &u);
};
