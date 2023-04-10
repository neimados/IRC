#include "../inc/Irc.hpp"

int main(int ac, char **av) {
    if (ac != 3 || !av[1][0] || !av[2][0]) {
        std::cout << "Error: usage : ./ircserv <port> <password>" << std::endl;
    }
    //Parsing dans la classe IRC avec check necessaire
    // Irc irc(argv[1], argv[2]);

    return 0;
}
