#include "parsing.hpp"
#include "WebServ.hpp"
#include "ConfigParser.hpp"
#include "server.hpp"

#include <csignal>

volatile sig_atomic_t isAlive = 0;

void sigint_handler(int signal)
{
	(void)signal;
	isAlive = 0;
}

int	main(int argc, char **argv)
{
	if (argc == 2)
	{
		ConfigParser	parser;

		try
		{
			parser.parseConfig(argv[1]);
			check_required(parser);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return (1);
		}

		int server_fd = start_server(*parser.getServer().begin());
		if (server_fd < 0) // Retourne une erreur si la page du server est ouverte, même non trouvée
		{
			std::cout << "Erreur a l'ouverture du server\n";
			return (1);
		}
		isAlive = 1;
		signal(SIGINT, sigint_handler);
		std::cout << "Socket écoute serveur: " << server_fd << "\n";

		// Après ctrl+c, refresh la page pour fermer proprement le prgrm (parce que pas encore de poll)
		while (isAlive)
		{
			int client_fd = accept(server_fd, NULL, NULL);
			if (client_fd == -1)
				continue;
			
			char buffer[4096];
			int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
			if (bytes > 0)
			{
				buffer[bytes] = '\0';

				std::string response =
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Length: 12\r\n"
					"\r\n"
					"Hello world\n";

				send(client_fd, response.c_str(), response.size(), 0);
			}

			close(client_fd);
		}
		close(server_fd);
	}
	return (0);
}