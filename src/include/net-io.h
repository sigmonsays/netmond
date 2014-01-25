#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


#include <netinet/in.h>

#include <netdb.h>

#include <stdio.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


char *net_request(int sock, char *req);
int wait_io(int mode, int sockfd, int timeout_sec, int timeout_usec);
