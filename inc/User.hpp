/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:48:25 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/15 00:34:20 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"

class Server;

class User {

public:
    User();
    User(pollfd Client, int fd);
    ~User();
    
    User    USER(std::string username);
    User    NICK(std::string nickname);

    // Getters
    std::string getUsername() const;
    std::string getNickname() const;
    std::string getPassword() const;

    int     getFd() const;
    bool    getVerification();
    bool    getUserVerification();
    bool    getNickVerification();
    bool    getChannelVerification();

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
    void    setNickVerification(bool type);
    void    setChannelVerification(bool type);
    void    setisInChannel(int n);
    void    setWhatChannel(std::string channelName);
    
    // Commands
    bool    channelLimit();

    void    increaseChannelNo();
    void    decreaseChannelNo();

private:
    pollfd      _client;
    int         _fd;
    std::string _username;
    std::string _nickname;
    std::string _password;

    bool        isVerified;
    bool        userIsVerified;
    bool        nickIsVerified;
    bool        isInChannel;

    std::string whatChannel;
    int noChannels;

    // User(const User &u);
    // User &operator=(const User &u);

};
