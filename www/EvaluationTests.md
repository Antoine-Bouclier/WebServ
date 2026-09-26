# Évaluation webserv

Commandes lancées depuis la racine du projet.

## 1. Compilation et lancement

```bash
make
./webserv config/default.conf
```

Attendu : compilation réussie ; le deuxième `make` ne recompile ni ne relie inutilement.

## 2. Deux sites

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8081/
```

Attendu : 200, avec un contenu différent pour chaque site.
Vérifier aussi dans le navigateur.

## 3. Pages d’erreur

```bash
curl -i http://127.0.0.1:8080/introuvable
curl -i http://127.0.0.1:8081/introuvable
```

Attendu :
- 8080 : 404 et page personnalisée.
- 8081 : 404 et page par défaut.

## 4. Route vers un autre répertoire et index

```bash
curl -i http://127.0.0.1:8080/docs/exemple.txt
curl -i http://127.0.0.1:8080/docs/
curl -i http://127.0.0.1:8080/docs
```

Attendu :
- Fichier : 200 avec son contenu.
- Répertoire avec index : 200 avec index.html.
- Sans slash final : 301 et Location: /docs/.

## 5. Autoindex

Renommer temporairement l’index :

```bash
mv www/documents/index.html www/documents/accueil.html
curl -i http://127.0.0.1:8080/docs/
```

Avec `autoindex off` : 403.

Passer à `autoindex on`, redémarrer puis refaire :

```bash
curl -i http://127.0.0.1:8080/docs/
```

Attendu : 200 et listing des fichiers.
Vérifier dans le navigateur que les liens fonctionnent.

Pour restaurer l’index :

```bash
mv www/documents/accueil.html www/documents/index.html
```

## 6. Méthodes interdites sur /docs

```bash
curl -i -X DELETE http://127.0.0.1:8080/docs/exemple.txt
curl -i -X POST -d "test" http://127.0.0.1:8080/docs/exemple.txt
curl -i http://127.0.0.1:8080/docs/exemple.txt
```

Attendu :
- DELETE et POST : 405 avec Allow: GET.
- GET : 200, fichier toujours intact.

## 7. Redirection configurée

```bash
curl -i http://127.0.0.1:8080/old
```

Attendu : 301 avec Location: /docs/.
Dans le navigateur : redirection vers /docs/.

## 8. Upload, récupération et suppression

Le fichier exemple.txt ne doit pas déjà exister dans www/uploads.

```bash
curl -i -F "file=@www/documents/exemple.txt" http://127.0.0.1:8080/upload
curl -i http://127.0.0.1:8080/files/exemple.txt
curl -i -X DELETE http://127.0.0.1:8080/files/exemple.txt
curl -i http://127.0.0.1:8080/files/exemple.txt
```

Attendu, dans l’ordre : 201, 200 avec le contenu original, 204, 404.

## 9. Limite du corps

Avec une limite de 1M sur /upload :

```bash
dd if=/dev/zero of=/tmp/webserv-too-large.bin bs=1024 count=2048
curl -i -F "file=@/tmp/webserv-too-large.bin" http://127.0.0.1:8080/upload
curl -i http://127.0.0.1:8080/
```

Attendu : 413 pour l’upload, aucun fichier enregistré, puis 200 sur le site.

## 10. Méthode inconnue

```bash
curl -i -X UNKNOWN http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/
```

Attendu : 501, puis 200. Le serveur reste actif.

## 11. CGI : GET et POST

```bash
curl -i "http://127.0.0.1:8080/cgi-bin/echo.py?nom=test"
curl -i -X POST -d "bonjour" http://127.0.0.1:8080/cgi-bin/echo.py
curl -i -X POST -d "bonjour" "http://127.0.0.1:8080/cgi-bin/echo.py?nom=test&age=42"
```

Attendu : 200 ; méthode, query et corps correspondent à la requête.

## 12. CGI : répertoire de travail

```bash
curl -i http://127.0.0.1:8080/cgi-bin/relative.py
```

Attendu : 200 avec « Lecture relative OK », lu depuis message.txt.

## 13. CGI : erreur et timeout

```bash
curl -i http://127.0.0.1:8080/cgi-bin/error.py
curl -i http://127.0.0.1:8080/cgi-bin/loop.py
```

Attendu :
- Script en erreur : 502.
- Boucle infinie : 504 après environ 10 secondes.

Pendant l’attente, dans un autre terminal :

```bash
curl -i http://127.0.0.1:8080/
```

Attendu : 200 sans attendre la fin du CGI.
Le traceback Python dans le terminal du serveur est normal.

## 14. Port déjà occupé

Garder le serveur actif et lancer dans un autre terminal :

```bash
./webserv config/default.conf
```

Attendu : erreur de bind pour la deuxième instance.
La première instance doit continuer de répondre.

## 15. Doublon dans la configuration

Arrêter le serveur.
Dans une copie de la configuration, mettre les deux serveurs sur
127.0.0.1:8080, puis lancer cette copie.

Attendu : rejet du doublon de configuration.

## 16. Plusieurs adresses d’écoute

Configurer :
- Site A : 127.0.0.1:8080
- Site B : 127.0.0.2:8081

Redémarrer puis lancer depuis WSL :

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.2:8081/
```

Attendu : 200 avec les contenus respectifs.
L’accès à 127.0.0.2 depuis un navigateur Windows dépend du réseau WSL.

## 17. Siege

Créer une page vide :

```bash
touch www/site-a/empty.html
```

Test court :

```bash
siege -b -c 20 -r 100 http://127.0.0.1:8080/empty.html
```

Test prolongé :

```bash
siege -b -c 20 -t 2M http://127.0.0.1:8080/empty.html
```

Attendu : disponibilité supérieure à 99,5 %, serveur toujours accessible.

Résultats déjà obtenus :
- Court : 2 000 transactions, 100 % de disponibilité, aucun échec.
- Prolongé : environ 2,83 millions de transactions en 123,62 secondes,
  100 % de disponibilité, aucun échec.

## 18. Valgrind

Arrêter l’instance normale, recompiler puis lancer :

```bash
make
valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./webserv config/default.conf
```

Dans un autre terminal, effectuer les requêtes GET, upload, DELETE et CGI.

Arrêter webserv avec Ctrl+C, puis lire le bilan :

```bash
tail -n 20 valgrind.log
```

Vérifier :
- ERROR SUMMARY: 0 errors
- definitely lost: 0
- indirectly lost: 0
- possibly lost: 0
- Nettoyage des allocations du projet à l’arrêt.

Le premier rapport, avant gestion de SIGINT, indiquait zéro erreur
et zéro bloc perdu, mais 89 215 octets encore accessibles.
Le résultat après ajout de l’arrêt propre reste à relever.