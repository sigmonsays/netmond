#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netinet/in.h>

#include <stdio.h>
#include <unistd.h>

#ifndef HAVE_SQL_SUPPORT_H
#include "sqlSupport.h"
#endif

/* ip and port of archnet web-based paging service 
	www.arch.com
*/
#define DEST_IP   "147.187.10.22"
#define DEST_PORT 80

int send_page(char *number, char *message);
void send_email(char *rcpt, char *msg);
void send_notifications(char *hostname, int port, int state);
int wants_notification(char *hostname, int pagerid);

