#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/types.h>  /* for Socket data types */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <netinet/in.h> /* for IP Socket data types */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <fcntl.h>

void _help (void);
void _quit (void);
void _reload (void);

int clientSend(int c, char *msg);
int clientRequest(int client, struct sockaddr_in cl_addr, int clinum);
