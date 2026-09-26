with open("message.txt") as fichier:
    contenu = fichier.read()

print("Content-Type: text/plain")
print()
print(contenu)