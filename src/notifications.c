#include "common.h"
#include "notifications.h"
#include "logging.h"

extern struct mconfig *main_config;

#ifdef ARCH_HTTP_PAGE
/* send page to number with message, 
	returns -1 on error, 0 on sucess
*/
int send_page(char *number, char *message) {
        int sockfd;
        struct sockaddr_in dest_addr;
        char *buf;
        int l, s;
                
        buf = (char *) malloc(256 + strlen(message));
                        
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
         
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(DEST_PORT);
        dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
        memset(&(dest_addr.sin_zero), '\0', 8);

        if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1) return -1;
        
        l = strlen(message) + strlen(number) + 13;

        sprintf(buf,    "POST /cgi-bin/archepage.exe HTTP/1.0\n"
                        "Content-type: application/x-www-form-urlencoded\n"
                        "Content-length: %d\n\n"
                        "ACCESS=%s&MSSG=%s\n\n", l, number, message);
        
        s = send(sockfd, buf, strlen(buf), 0);
        shutdown(sockfd, 2);
        close(sockfd);
	free(buf);
	return 0;
}
#endif

void send_email(char *rcpt, char *msg) {
        FILE *p;
        char *cmd;

        cmd = (char *) malloc(strlen(rcpt) + 28);
        sprintf(cmd, "sendmail -t -i '%s'", rcpt);

        p = popen(cmd, "w");

        if (p != NULL) {
                fputs(msg, p);
                fflush(p);
                pclose(p);
        }
	free(cmd);
}

/* send notifcations out to all people(s)
	state is the 1 (up) or 0 (down), -1 error
*/
void send_notifications(char *hostname, int port, int state) {
	char buffer[255];
	MYSQL *mysqlConnection;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char qry[1024], buf[128];
	int ql;

	mysqlConnection = init_sql();
	if (!mysqlConnection) return;

	sprintf(buf, "%s %d %s", hostname, port, ((state == 1) ? "up" : "down") );
	strcpy(qry, "SELECT pager,send_page,email,send_email,id FROM notifications");
	ql = strlen(qry);

	if (mysql_real_query(mysqlConnection, qry, ql)) return;

	if (! (res = mysql_store_result(mysqlConnection)) ) return;

	while ((row = mysql_fetch_row(res))) {

		if (wants_notification(hostname, atoi(row[4]))) {

#ifdef ARCH_HTTP_PAGE
			if (strcmp(row[1], "1") == 0) { /* send page */
				/* check if they want to receive pages for this server */
				sprintf(buffer, " sending page to %s...\n", row[0]);
				_log(buffer, 1);
				send_page(row[0], buf);
			}
#endif

			if (strcmp(row[3], "1") == 0) { /* send e-mail */
				sprintf(buffer, " sending email to %s...\n", row[2]);
				_log(buffer, 1);
				send_email(row[2], buf);
			}
		}

	}

	mysql_free_result(res);
	close_sql(mysqlConnection);
}

/* function to check if (pagerid) wants to be notified of server (hostname)

	returns 1 if they do, 0 if they do not, -1 on error
*/
int wants_notification(char *hostname, int pagerid) {
	MYSQL *mysqlConnection;

	MYSQL_RES *res;
	MYSQL_ROW row;
	char qry[1024];
	int ql;
	my_ulonglong num_rows;
	char *ids, *tmp;

	int rv = 0;

	mysqlConnection = init_sql();
	if (!mysqlConnection) return -1;

	sprintf(qry, "SELECT contactHosts.contactids FROM contactHosts,servers WHERE contactHosts.sid = servers.id AND servers.host='%s'", hostname);
	ql = strlen(qry);

	if (mysql_real_query(mysqlConnection, qry, ql)) return -1;

	if (! (res = mysql_store_result(mysqlConnection)) ) return -1;

	num_rows = mysql_num_rows(res);

	if (num_rows) {
		row = mysql_fetch_row(res);
		ids = strdup(row[0]);

		tmp = strtok(ids, ",");
		if (tmp) {
			while (1) {
				if (atoi(tmp) == pagerid) {
					rv = 1;
					break;
				}
				if (!(tmp = strtok(NULL, ","))) break;
			}
		}

		free(ids);
	} else {
		// we will assume if they don't have a record that they want to be notified
		rv = 1;
	}

	mysql_free_result(res);

	close_sql(mysqlConnection);

	return rv;
}
