// version 1.0.0
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "tcpserver.h"

/*
Action on SIGCHLD to manage Child termination
*/
void received_sigchld_on_tcpserver(int sig) {
	int status,pid;
	//wait for one child process ends
	pid=wait(&status);
	printf("wait pid=%d status=%d\n",pid,status);
}

void listen_sigchld() {
	struct sigaction eventSigChld;
	//Create the sigaction structure to handle SIGCHLD signal
	sigemptyset(&eventSigChld.sa_mask);
	eventSigChld.sa_flags=0;
	eventSigChld.sa_handler= received_sigchld_on_tcpserver;
	sigaction(SIGCHLD,&eventSigChld, NULL);
}

int listen_request(unsigned int port, unsigned int pool_size, connection_handler handler) {
	int mainfd,clientfd;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t clientsize;
	pid_t     childpid,serverpid;

	mainfd=socket(AF_INET,SOCK_STREAM,0);

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	server.sin_port=htons(port);
	bind(mainfd,(struct sockaddr *)&server,sizeof(server));

	listen(mainfd,pool_size);
	listen_sigchld();
	childpid=-1;
	serverpid=getpid();
	while (serverpid==getpid()) {
		bzero(&client,sizeof(client));
		clientsize=sizeof(client);
		clientfd = accept(mainfd,(struct sockaddr *)&client,&clientsize);
		//printf("%d CHILDPID : %d clientfd=%d mainfd=%d errno=%d\n",getpid(),childpid, clientfd, mainfd, errno);
		//When we receive SIGCHLD the access is unlock and return -1, ernno=4 (EINTR)
		if (clientfd>0) {
			if ((childpid = fork()) == 0) {
				close (mainfd);
				handler(clientfd,client, clientsize);
			}
			//printf("%d close clientfd %d\n", getpid(), clientfd);
			close(clientfd);
		}
	}
	return 1;
}
