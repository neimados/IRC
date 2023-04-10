#include "../inc/Irc.hpp"

int main(int ac, char **av) {
    if (ac != 3 || !av[1][0] || !av[2][0]) {
        std::cout << "Error: usage : ./ircserv <port> <password>" << std::endl;
    }

    //Gestion des signaux, de memoire :
    //signal(SIGINT, appel_de_fonction); CTRL C
    //signal(SIGTERM, appel_de_fonction); CTRL D

    try {
        //Parsing dans la classe IRC avec check necessaire
        //Une grosse classe qui va contenir des objets des autres classes
        // Irc irc(argv[1], argv[2]);

        //On cree le serveur
        //ir.init();

        //On le lance
        //irc.launch();c

        //LOOP pour recup input genre:
        //while (not ctrl d) ...
    } catch (std::exception &e) {
        std::cout<<e.what()<<std::endl;
        return 1;
    }

    return 0;
}
