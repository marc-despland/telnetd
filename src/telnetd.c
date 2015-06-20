#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "termtty.h"
#include "tcpserver.h"
#include "options.h"

#define MDE_TELNETD_VERSION "1.1.0"

void version(char * cmd) {
	printf("Telnetd version %s\n",MDE_TELNETD_VERSION);
	printf("https://github.com/marc-despland/samples\n\n");
}


int handler(int clientfd, struct sockaddr_in client, socklen_t clientsize) {
	char address[INET6_ADDRSTRLEN];


	inet_ntop(AF_INET, &(client.sin_addr), address, INET6_ADDRSTRLEN );
	printf("Coonection established from %s\n",address);
	send(clientfd,"You are connected to a test telnet server\n", 42, 0);
	send(clientfd,"Send CTRL+D to end the connection\n\n", 35, 0);
	start_terminal(clientfd, clientfd);
	return 1;
}

int main(int argc, char **argv) {
	unsigned int port=0;
	unsigned int pool=12;
	Option options[]={
			{'n',"poolsize","The number of simultaneous clients that can connect", TRUE,FALSE,FALSE,&pool, INT},
			{'p',"port","The port to listen to", TRUE,TRUE,FALSE,&port, INT}
	};
	int result= parse_options(argc, argv, options, 2, version);
	if (result<0) {
		return 0;
	} else {
		listen_request(port,pool,&handler);
		return 1;
	}
}
