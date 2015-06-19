#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "termtty.h"
#include "tcpserver.h"

int handler(int clientfd, struct sockaddr_in client, socklen_t clientsize) {
	int go=1;
	int n;
	char buffer[100];
	char address[INET6_ADDRSTRLEN];


	inet_ntop(AF_INET, &(client.sin_addr), address, INET6_ADDRSTRLEN );
	printf("Coonection established from %s\n",address);
	send(clientfd,"You are connected to a test telnet server\n", 42, 0);
	send(clientfd,"Send CTRL+D to end the connection\n\n", 35, 0);
	start_terminal(clientfd, clientfd);
	return 1;
}

int main() {

	listen_request(6666,12,&handler);
    return 1;
}
