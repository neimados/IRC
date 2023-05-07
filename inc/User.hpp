/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:48:25 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/08 00:27:12 by dvergobb         ###   ########.fr       */
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

    // Getters
    std::string getUsername() const;
    std::string getNickname() const;
    std::string getPassword() const;
    std::string getHostname() const;
    std::string getMode() const;

    int     getFd() const;
    int     getSocket() const;
    bool    getVerification();
    bool    getUserVerification();
    bool    getChannelVerification();
    bool    getPassOk();
    bool    getIsOperator() const;
    bool    getIsVoiced() const;

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
    void    setIsOperator(bool isOperator);
    void    setIsVoiced(bool isVoiced);

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

    bool        _isOperator;
    bool        _isVoiced;

    std::string whatChannel;
    int noChannels;

    // User(const User &u);
    // User &operator=(const User &u);
};
