# Run
- make test
- Or
- make && ./ircserv "port" "password"

# Example of use (in other terminal)
- Run: nc 127.0.0.1 6667

# Ressources
- https://tools.ietf.org/html/rfc1459

# Commandes à implémenter
- ✅ NICK
- ✅ PASS
- ✅ USER
- ✅ QUIT
- ✅ LIST          (channel)
- ✅ JOIN          (channel)
- ✅ PART          (channel)
- ✅ NAMES         (channel)
- ✅ TOPIC         (channel)
- ✅ KICK          (channel)
- ✅ PRIVMSG       (message)
- ✅ NOTICE        (message)
- ✅ MODE          (channel)

✅ <!-- class Chanel  -->
+o : accorder l'opérateur de chaîne à un utilisateur
-o : retirer l'opérateur de chaîne à un utilisateur

✅ <!-- class Chanel  -->
+v : donner le droit de voix à un utilisateur sur un canal
-v : retirer le droit de voix à un utilisateur sur un canal

✅ <!-- PRIVMSG NOTICE cmd -->
+m : mode modéré (seuls les opérateurs de chaîne et les utilisateurs avec la voix peuvent envoyer des messages sur le canal)
-m : supprimer le mode modéré

✅ <!-- PRIVMSG NOTICE cmd -->
+n : mode interdire les messages externes (seuls les utilisateurs sur le canal peuvent envoyer des messages sur le canal)
-n : supprimer le mode interdire les messages externes

✅ <!-- LIST cmd -->
+p : mode privé (le canal ne sera pas visible par la commande LIST)
-p : supprimer le mode privé

✅ <!-- TOPIC cmd -->
+t : mode limiter les sujets (seuls les opérateurs de chaîne peuvent changer le sujet)
-t : supprimer le mode limiter les sujets



## À corriger

Codes d'envoi de messages
Réponses et messages d'erreur
Mode -i pour INVITE
User : get and update nickname
✅ Message du QUIT
✅ PASS, NICK puis USER
✅ Notice et Privmsg : gérer les multiples destinataires
✅ PASS avec ctrl + D