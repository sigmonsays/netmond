#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <sys/signal.h>

#include <netinet/in.h>

#include <netdb.h>

#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <math.h>

#include "netmond.h"

int check_port(char *host, int port);
int check_servers(int signum);
int get_port_status(int hostid, int port_num);
int set_port_status(int hostid, int port_num, int new_status);

