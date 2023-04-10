int main(int ac, char **av) {
    if (ac != 3 || !av[1][0] || !av[2][0]) {
        std::cout << "Error: usage : ./ircserv <port> <password>" << std::endl;
    }
    return 0;
}
