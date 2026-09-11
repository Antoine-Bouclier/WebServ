# Webserv — feuille de route mandatory

Référence : en.subject.webserv.pdf, version 24.1. Les pages ci-dessous sont les numéros imprimés du sujet, pas les numéros du lecteur PDF.

Audit actualisé du 10 septembre 2026, branche `main`, commit `1b10ad3`. Les ajouts locaux DELETE du premier audit ne sont plus présents. Aucun bonus. Les règles générales et le README requis restent applicables.

Contraintes de travail : C++98, pas de namespace ajouté, pas de friend, pas d'allocation de style C. Signatures de fonctions sur une seule ligne. Code concis, sans architecture supplémentaire sans besoin concret.

## 1. Sécuriser et unifier les chemins

- Séparer chemin et query ; utiliser le chemin pour les fichiers.
- Retirer le préfixe de location conformément à l'exemple `/kapouet` du sujet p. 10.
- Définir le décodage des URL et empêcher les sorties du répertoire configuré, y compris par liens symboliques.
- Partager cette résolution entre GET, DELETE, uploads et CGI.
- Validation : query sur fichier existant, route avec root distinct, traversées refusées, aucun DELETE hors root.

## 2. Relier chaque connexion à sa configuration

- Sélectionner le serveur à partir du listener qui a accepté la connexion.
- Sélectionner la location après les en-têtes, avant de valider/recevoir le corps.
- Appliquer la limite effective de corps et les bonnes pages d'erreur.
- Retirer le routage par nom si l'on choisit de supprimer les virtual hosts facultatifs.
- Validation : deux ports avec contenus et limites distincts ; limite locale effectivement appliquée.

## 3. Terminer le parsing HTTP

- Initialiser le chunked en lecture de taille ; vérifier tailles, séparateurs et fragmentation.
- Donner une signification cohérente à une limite de corps nulle.
- Conserver, initialiser et exposer le statut d'erreur du parseur ; le copier correctement.
- Borner la ligne de requête et les en-têtes pendant leur réception.
- Libérer les octets déjà consommés plutôt que conserver une deuxième copie de tout le corps.
- Validation : Content-Length et chunked, requêtes fragmentées, corps binaires, dépassements, erreurs distinctes de 400.

## 4. Fiabiliser la boucle serveur

- Ajouter des délais pour requête incomplète et client qui ne lit plus.
- Contrôler les échecs de configuration des sockets et nettoyer les ressources.
- Traiter les erreurs propres à une connexion sans terminer tout le serveur.
- Éviter la copie des objets propriétaires de sockets.
- Garder un seul poll pour sockets et futurs pipes CGI, avec lecture/écriture selon les besoins.
- Ne pas consulter errno après read/recv/write/send pour ajuster le comportement.
- Validation : clients lents, déconnexions, envois partiels et connexions concurrentes ; serveur toujours disponible.

## 5. Finaliser les réponses statiques et DELETE

- Sécuriser fichiers vides, échecs tellg/read et tailles importantes.
- Vérifier index, autoindex, types MIME, pages d'erreur et restrictions de méthodes.
- Échapper le HTML de l'autoindex et encoder ses liens.
- Implémenter DELETE et les réponses associées après sécurisation des chemins ; ajouter Allow aux 405.
- Validation : site complet au navigateur, fichiers binaires et vides, index activé/désactivé, DELETE autorisé/interdit.

## 6. Ajouter les redirections configurables

- Ajouter une directive de route, son stockage et une réponse avec statut et Location.
- Validation : redirection suivie par le navigateur et indépendante de l'existence du fichier local.

## 7. Implémenter POST et l'upload

- Faire respecter l'autorisation d'upload et upload_path.
- Choisir un format de dépôt documenté ; prendre en charge le formulaire de démonstration, multipart si utilisé.
- Écrire les octets reçus, sécuriser le nom et définir la politique d'écrasement.
- Nettoyer les fichiers incomplets en cas d'erreur.
- Validation : contenu réellement présent sur disque, fichier binaire, limite dépassée, destination interdite, upload puis GET/DELETE.

## 8. Implémenter un seul type de CGI

- Choix proposé : Python, déjà présent dans la configuration.
- Préparer environnement, query, corps décodé, répertoire de travail et execve.
- Intégrer les pipes non bloquants au poll existant, fermer stdin à la fin du corps.
- Lire la sortie jusqu'à EOF si nécessaire, interpréter ses en-têtes et son statut.
- Ajouter délai, arrêt du processus, waitpid sans bloquer et nettoyage de tous les descripteurs.
- Validation : GET/query, POST binaire et chunked, sortie sans Content-Length, script lent, échec et requêtes concurrentes.

## 9. Nettoyer et préparer l'évaluation

- Supprimer code mort et commentaires obsolètes ; conserver les tests utiles.
- Rejeter proprement les configurations tronquées, y compris les directives dans un mauvais contexte.
- Fournir des configurations et fichiers de démonstration pour chaque fonctionnalité.
- Corriger le README en anglais selon le chapitre V : première ligne, Description, Instructions, Resources et usage de l'IA.
- Vérifier compilation complète C++98, règles Makefile et absence de relink inutile.
- Tester navigateur, client HTTP brut, charge et gestion des ressources.

## Résultats de départ

- Compilation réussie ; second make sans relink.
- Tests existants : 5/7. Méthode non supportée : 400 au lieu de 501 ; POST : 200 vide sans stockage.
- Tests isolés : GET avec query échoue ; root de location mal mappé ; second port sert le premier site ; chunked valide rejeté ; limite locale ignorée ; dépassement global rendu en 400.
- Traversée GET confirmée : lecture d'un fichier temporaire hors root. DELETE renvoie désormais 200 sans supprimer le fichier : son implémentation du premier audit n'est plus présente.
- Requête incomplète : délai du client de test atteint ; aucun mécanisme de délai serveur dans le code.
- Fichier vide : réponse 200 observée, mais accès à buffer[0] sur vecteur vide à corriger.

Les tests initiaux ne constituent pas une validation de charge, de mémoire ou de compatibilité navigateur complète.

## Mise à jour — 11 septembre 2026

- Chemins : séparation query, mapping des locations, décodage unique et refus des segments parents réalisés. Liens symboliques hors root non traités.
- Serveurs : sélection par listener, plusieurs interfaces/ports testés ; virtual hosts retirés du routage.
- Corps : configuration de location choisie avant validation ; limite conservée entre les lectures.
- Chunked simple : reconstruction, fragmentation, débordements et limite effective testés. Extensions et trailers non vides refusés par cette version.
- Validation : 30 tests chemins, 21 listeners, 14 limites Content-Length, 14 limites chunked via HTTP ; 15 scénarios directs du parseur avec coupures et vérification du corps. Suite générale 6/7 (POST absent).
- Prochaine priorité : délais clients et robustesse réseau/mémoire. Restent DELETE, upload, redirections, CGI unique, finitions et validation finale.
