/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:51 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/17 22:05:00 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

// Function to catch ctrl + c
void    sigint_handler(int sig)
{
    (void)sig;
    
    std::cout << std::endl << ORANGE << BOLD;
    std::cout << "Stop listenning, close IRC.";
    std::cout << RESET << std::endl;
    exit(0);
}

// Function to catch ctrl + d
void    sigterm_handler(int sig)
{
    (void)sig;

    // Clear console
    std::cout << "\033[2J\033[1;1H";
    
    std::cout << std::endl << GREY << ITALIC;
    std::cout << "IRC terminal was cleared.";
    std::cout << RESET << std::endl;
    exit(0);
}

int main(int ac, char **av) {
    if (ac != 3 || !av[1][0] || !av[2][0]) {
        std::cout << "Error: usage : ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(av[1]);

    // Catch ctrl + c
    signal(SIGINT, sigint_handler);
    // Catch ctrl + d
    signal(SIGTERM, sigterm_handler);
    

    try {
        if (port >= 0 && port <= 65535) {
            // Parsing dans la classe IRC avec check necessaire
            // Une grosse classe qui va contenir des objets des autres classes
            // Irc irc(port, av[2]);

            // classe serveur test
            Server srv(port);
            
            srv.startSrv();
        } else {
            std::cout << "Error: port must be between 0 and 65535" << std::endl;
            return 1;
        }
    } catch (std::exception &e) {
        std::cout<<e.what()<<std::endl;
        return 1;
    }

    return 0;
}
