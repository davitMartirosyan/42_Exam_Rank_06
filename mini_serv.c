#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>



typedef struct t_cl
{
	int id;
	int fd;
	struct t_cl* next;
} clt;

clt *clients = NULL;
fd_set current, read_set, write_set;
int sockfd, g_id;
char msg[200000], buff[200000];



void fatal( void )
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(sockfd);
	exit(1);
}

int get_max( void )
{
	int max = sockfd;
	clt* tmp = clients;
	while (tmp)
	{
		if (max > tmp->fd)
			max = tmp->fd;
		tmp = tmp->next;
	}
	return (max);
}

int getid(int fd)
{
	clt *tmp = clients;
	while (tmp)
	{
		if (tmp->fd == fd)
			return (tmp->id);
		tmp = tmp->next;
	}
	return (-10);
}

void add( void )
{
	clt *tmp = clients;
	clt *new;
	int client_fd;

	struct sockaddr_in claddr;
	socklen_t len = sizeof(claddr);
	client_fd = accept(sockfd, (struct sockaddr*)&claddr, &len);
	if (client_fd < 0)
		fatal();
	memset(&new, 0, sizeof(new));
	sprintf(buff, "server: client %d just arrived\n", g_id);
	FD_SET(client_fd, &current);
	new = malloc(sizeof(clt));
	if (!new)
		fatal();
	new->fd = client_fd;
	new->id = g_id++;
	new->next = NULL;
	if (!clients)
		clients = new;
	else
	{
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = new;
	}	
}



int main(int ac, char **av)
{
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}
	struct sockaddr_in srvaddr;
	memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_addr.s_addr = htonl(2130706433);
	srvaddr.sin_port = htons(atoi(av[1]));

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		fatal();
	if ((bind(sockfd, (struct sockaddr*)&srvaddr, sizeof(srvaddr)) < 0))
		fatal();
	if ((listen(sockfd, 100) < 0))
		fatal();
	FD_ZERO(&current);
	FD_SET(sockfd, &current);
	memset(&msg, 0, sizeof(msg));

	while (1)
	{
		read_set = write_set = current;
		if (select(get_max() + 1, &read_set, &write_set, NULL, NULL) < 0)
			continue;
		for(int fd = 0; fd <= get_max(); fd++)
		{
			if (FD_ISSET(fd, &read_set))
			{
				if (fd == sockfd)
				{
					add();
					break;
				}
			}
		}
	}
	return (0);
}
