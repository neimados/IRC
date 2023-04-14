# Run:
- make test
- Or
- make && ./ircserv <port> <password>

# Example of use (in other terminal)
- Run: nc 127.0.0.1 6667

# Ressources
- https://tools.ietf.org/html/rfc1459

# Commandes à implémenter
- /join #nomducanal : rejoint le canal spécifié
- /part #nomducanal : quitte le canal spécifié
- /nick pseudonyme : change votre pseudonyme
- /msg utilisateur message : envoie un message privé à l'utilisateurspécifié
- /quit : quitte le serveur IRC
- /whois utilisateur : affiche des informations sur l'utilisateur spécifié
- /topic #nomducanal sujet : définit le sujet du canal spécifié

# Commandes à implémenter (bonus)
- /ignore utilisateur : ignore les messages de l'utilisateur spécifié
- /unignore utilisateur : arrête d'ignorer les messages de l'utilisateur spécifié
- /list : affiche la liste des canaux
- /names #nomducanal : affiche la liste des utilisateurs du canal spécifié
- /invite utilisateur #nomducanal : invite l'utilisateur spécifié dans le canal spécifié
- /kick #nomducanal utilisateur : expulse l'utilisateur spécifié du canal spécifié
- /mode #nomducanal +o utilisateur : donne le statut d'opérateur à l'utilisateur spécifié
- /mode #nomducanal -o utilisateur : retire le statut d'opérateur à l'utilisateur spécifié
- /mode #nomducanal +v utilisateur : donne le statut de voix à l'utilisateur spécifié
- /mode #nomducanal -v utilisateur : retire le statut de voix à l'utilisateur spécifié

# Liste des statuts d'opérateur
- o : opérateur
- v : voix
