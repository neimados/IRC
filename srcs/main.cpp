#include "../inc/Irc.hpp"
#include "../inc/Server.hpp"

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
        Irc irc(av[1], av[2]);

        //classe serveur test
        Server srv;
        srv.startSrv();

    } catch (std::exception &e) {
        std::cout<<e.what()<<std::endl;
        return 1;
    }

    return 0;
}
