#include <fcntl.h>
#include <sys/socket.h>

/*
Creates a socket, and returns its file descriptor.
*/
int createSocket(bool non_block)
{
	// AF_INET = IPv4 | SOCK_STREAM = TCP
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		// Mettre un strerror ici
		return (-1);
	}

	// F_SETFL = fd set flags | O_NONBLOCK = nonblock mode
	if (non_block)
	{
		int fd_flags = fcntl(socket_fd, F_GETFL, 0);
		fcntl(socket_fd, F_SETFL, fd_flags | O_NONBLOCK);
	}

	return (socket_fd);
}


