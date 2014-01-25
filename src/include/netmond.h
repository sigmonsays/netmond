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

/* default values */
#define MAX_CLIENTS 8
#define BIND_ADDR INADDR_ANY
#define PORT 2421

/*
 *	define this if you want to be able to use host names in sql, this sometimes causes problems
 *	when gethostbyname returns "resource temp. unavail.".
 */

// #define USE_GETHOSTBYNAME

#define VER_MAJOR 0
#define VER_MINOR 1
#define VER_MICRO 0

// #define ARCH_HTTP_PAGE

void _help (void);
void _quit (void);
void _reload (void);

int clientSend(int c, char *msg);
int clientRequest(int client, struct sockaddr_in cl_addr, int clinum);

