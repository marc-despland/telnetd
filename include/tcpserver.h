#ifndef _TCPSERVER_H
#define _TCPSERVER_H
#include <sys/socket.h>
typedef int (*connection_handler)(int, struct sockaddr_in , socklen_t);
int listen_request(unsigned int port, unsigned int pool_size, connection_handler handler);

#endif
