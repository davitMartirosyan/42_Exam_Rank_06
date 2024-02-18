#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <netdb.h>

int sockfd;
int clients = 65000;
int SIZE = 200000;

void fatal( void )
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(sockfd);
	exit(1);
}


int main(int ac, char** av)
{
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}
	struct sockaddr_in srv;
	fd_set active, ready, wr;
	int id = 0;
	int arr[clients];
	char buffer[SIZE];
	char msg[SIZE];
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		fatal();
	srv.sin_family = AF_INET;
	srv.sin_addr.s_addr = htonl(2130706433);
	srv.sin_port = htons(atoi(av[1]));
	if (bind(sockfd, (struct sockaddr*)&srv, sizeof(srv)) < 0) 
		fatal();
	if (listen(sockfd, 100))
		fatal();
	int maxfd = sockfd;
	FD_ZERO(&active);
	FD_SET(sockfd, &active);
	while (1)
	{
		ready = wr = active;
		if (select(maxfd + 1, &ready, &wr, NULL, NULL) < 0)
			continue;
		for(int fd = 0; fd <= maxfd; fd++)
		{
			if (FD_ISSET(fd, &ready))
			{
				bzero(&msg, strlen(msg));
				bzero(&buffer, strlen(msg));
				if (fd == sockfd)
				{
					//new client just arrived
					struct sockaddr_in claddr;
					socklen_t len = sizeof(claddr);
					
					int clt = accept(sockfd, (struct sockaddr*)&claddr, &len);
					if (clt < 0)
						continue;
					maxfd = (clt > maxfd) ? clt : maxfd;
					sprintf(buffer, "server: client %d just arrived\n", id);
					arr[clt] = id++;
					for(int j = 2; j <= maxfd; j++)
					{
						if (FD_ISSET(j, &wr) && j != sockfd)
						{
							if (send(j, buffer, strlen(buffer), 0) < 0)
								fatal();
						}
					}
					FD_SET(clt, &active);
				}
				else if (fd != sockfd)
				{
					//the client request
					int count = 1;
					while (count == 1 && msg[strlen(msg)-1] != '\n')
						count = recv(fd, msg + strlen(msg), 1, 0);
					if (count <= 0)
					{
						sprintf(buffer, "server: cleint %d just left", arr[fd]);
						FD_CLR(fd, &active);
						close(fd);
						for(int z = 2; z <= maxfd; z++)
						{
							if (FD_ISSET(z, &wr) && z != fd)
							{
								if (send(z, buffer, strlen(buffer), 0) < 0)
								{
									fatal();
								}
							}
						}
					}
					else
					{
						sprintf(buffer, "client %d: %s", arr[fd], msg);
						for(int z = 2; z <= maxfd; z++)
							if (FD_ISSET(z, &wr) && z != sockfd)
								if (send(z, buffer, strlen(buffer), 0) < 0)
									fatal();
					}
				}
				FD_CLR(fd, &ready);
			}
		}
	}
	return (0);	
}
