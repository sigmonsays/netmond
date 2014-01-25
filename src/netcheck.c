#include "common.h"
#include "netcheck.h"
#include "logging.h"

#include "sqlSupport.h"
#include "notifications.h"

#include "smtp-test.h"
#include "pop3-test.h"
#include "imap-test.h"
#include "rad-test.h"
#include "http-test.h"

extern int netcheck_running;
extern struct mconfig *main_config;

/* check port on host
	returns 1 if the port is open, 0 if the port is closed, -1 on error
*/
int check_port(char *host, int port) {
	int sockfd, r;
	struct sockaddr_in dest_addr;
	struct hostent *hp;
	fd_set rfds, wfds;
	struct timeval timeout;
	int rc;
	char tmp;

	memset(&dest_addr, 0, sizeof(struct sockaddr_in));

	if ((hp = gethostbyname(host)) == NULL) {
		return -1;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);


	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;

	/* this will usually return -1 on a non-blocking socket */
	connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

	timeout.tv_sec = 4;
	timeout.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);

	FD_ZERO(&wfds);
	FD_SET(sockfd, &wfds);

	r = select(sockfd + 1, &rfds, &wfds, NULL, &timeout);

	if (r == -1) {
		shutdown(sockfd, 2);
		close(sockfd);
		return -1;
	}

	if (r > 0) {
		if (FD_ISSET(sockfd, &rfds) || FD_ISSET(sockfd, &wfds)) {

			rc = 1;
			r = recv(sockfd, &tmp, 1, MSG_PEEK);
			if (r == -1) {
				if (errno == ECONNREFUSED) { // connection refused
					rc = 0;
				}
			}
			shutdown(sockfd, 2);
			close(sockfd);
			return rc;
		}
	}

	shutdown(sockfd, 2);
	close(sockfd);
	// default to port closed
	return 0;
}



/* main-loop check servers function (SIGALRM handler)
	returns 0 on success
*/
int check_servers(int signum) {

	char buffer[255];
	int r, i, status;
	MYSQL *mysqlConnection = NULL;
	struct host_ent *hostInfo;
	struct sql_data *sql_res;
	int *ports_list;
	int num_ports;
	int port_check;
	float f;
	int j;

	_log("\n\nCaught ALRM! -- Checking servers....\n", 0);

	if (netcheck_running == 1) {
		_log("check_servers: there appears to be a check already running, exiting..\n", 0);

		signal(signum, (void *) check_servers);
		alarm(main_config->interval);
		return -1;
	}

	netcheck_running = 1;

	if (!(mysqlConnection = init_sql())) {

		_log("check_servers: Failed to connect to sql\n", 0);
		netcheck_running = 0;
		signal(signum, (void *) check_servers);
		alarm(main_config->interval);
		return -1;
	}

	sql_res = (void *) malloc(sizeof(struct sql_data));

	r = enumerate_hosts(mysqlConnection, NULL, sql_res);
	if (r) {
		_log("check_servers: Failed to get hosts list\n", 0);

		free(sql_res);
		netcheck_running = 0;
		signal(signum, (void *) check_servers);
		alarm(main_config->interval);
		return 1;
	}

	hostInfo = (void *) malloc(sizeof(struct host_ent));

	while (enumerate_hosts(mysqlConnection, hostInfo, sql_res) == 0) {


		sprintf(buffer, "Checking host %s (%s)..\n", hostInfo->hostname, hostInfo->ports);
		_log(buffer, 1);

		num_ports = 64;

		ports_list = parse_ports(hostInfo->ports, &num_ports);

		for(i=0; i<num_ports; i++) {

			status = get_port_status(hostInfo->id, ports_list[i]);

			/* specialized service checking */

			if (ports_list[i] == 1812) { //radius
				port_check = (check_rad(hostInfo->hostname, main_config->rad->secret) == 0) ? 1 : 0;

			} else if (ports_list[i] == 25) {
				port_check = (check_smtp(hostInfo->hostname) == 0) ? 1 : 0;

			} else if (ports_list[i] == 80) {
				port_check = (check_http(hostInfo->hostname) == 0) ? 1 : 0;

			} else if (ports_list[i] == 110) {
				port_check = (check_pop3(hostInfo->hostname) == 0) ? 1 : 0;

			} else if (ports_list[i] == 143) {
				port_check = (check_imap(hostInfo->hostname) == 0) ? 1 : 0;

			} else {
				port_check = check_port(hostInfo->hostname, ports_list[i]);
			}

			if (port_check == 0) { /* port is closed */

				status++;
				sprintf(buffer, " port %d closed, status %d\n", ports_list[i], status);
				_log(buffer, 1);

				if (status >= 2) {
					/* oh shit, the fucker is down (been down for interval * 2)
						send out notifcations
					*/

					f = (float) sqrt((double) status);
					j = (int) f;
					if ((f == j) || (status == 2)) {
						backlog(hostInfo->hostname, ports_list[i], 2);

						if (main_config->notifications == 1)
							send_notifications(hostInfo->name, ports_list[i], 0);
					}
				}
				set_port_status(hostInfo->id, ports_list[i], status);

			} else if (port_check == 1) { /* the port is open.. */

				if (status >= 2) {
					/* port was closed, now it's open ... tell the mofos */
					sprintf(buffer, " port %d is now open\n", ports_list[i]);
					_log(buffer, 1);

					backlog(hostInfo->hostname, ports_list[i], 1);

					if (main_config->notifications == 1)
						send_notifications(hostInfo->name, ports_list[i], 1);

				} else {
					sprintf(buffer, " port %d open\n", ports_list[i]);
					_log(buffer, 1);
				}

				status = 0;
				set_port_status(hostInfo->id, ports_list[i], status);

			} else { /* error in check port */
				sprintf(buffer, "Error checking port %d for %s\n", ports_list[i], hostInfo->hostname);
				_log(buffer, 1);
			}
		}
		if (ports_list) {
			free(ports_list);
			ports_list = NULL;
		}
		if (hostInfo->ports) {
			free(hostInfo->ports);
			hostInfo->ports = NULL;
		}
		if (hostInfo->hostname) {
			free(hostInfo->hostname);
			hostInfo->hostname = NULL;
		}
		if (hostInfo->hostname) {
			free(hostInfo->name);
			hostInfo->name = NULL;
		}

	}

	free(sql_res);
	free(hostInfo);

	close_sql(mysqlConnection);

	/* setup the signal again */
	signal(signum, (void *) check_servers);
	alarm(main_config->interval);

	/* we're not running anymore */
	netcheck_running = 0;

	time(&main_config->last_check);

	_log("check finished\n", 0);

	return 0;
}


/* function that sets the status of a given port on host
	returns -1 on error, otherwise 0
*/
int set_port_status(int hostid, int port_num, int new_status) {

	MYSQL *mysqlConnection;

	char qry[1024];
	int ql;
	my_ulonglong afro;

	mysqlConnection = init_sql();
	if (!mysqlConnection) return -1;

	if (new_status == 0) {
		ql = sprintf(qry, "DELETE FROM status WHERE serverid='%d' AND port='%d'", hostid, port_num);
		if (mysql_real_query(mysqlConnection, qry, ql)) {
			close_sql(mysqlConnection);
			return -1;
		}
	} else {
		ql = sprintf(qry, "UPDATE status SET status = '%d' WHERE serverid='%d' AND port='%d'", new_status, hostid, port_num);

		if (mysql_real_query(mysqlConnection, qry, ql)) {
			close_sql(mysqlConnection);
			return -1;
		}

		afro = mysql_affected_rows(mysqlConnection);

		if ((int)afro == 0) { /* if we don't have an afro we need one */
			sprintf(qry, "INSERT INTO status VALUES('%d', '%d', '%d')", hostid, port_num, new_status);
			ql = strlen(qry);
			if (mysql_real_query(mysqlConnection, qry, ql)) {
				close_sql(mysqlConnection);
				return -1;
			}
		}
	}
	close_sql(mysqlConnection);

	return 0;
}


/* function that returns that status of a given port on a host
       -1 - error
	0 - open
	1 - warning
	2 - send notifcations
*/

int get_port_status(int hostid, int port_num) {
	MYSQL *mysqlConnection;

	MYSQL_RES *res;
	MYSQL_ROW row;

	char qry[1024];
	int ql;
	int current_status;
	my_ulonglong num_rows;


	mysqlConnection = init_sql();
	if (!mysqlConnection) {
		_log("ERROR: Failed to init_sql()\n", 0);
		return -1;
	}

	sprintf(qry, "SELECT `status` FROM `status` WHERE serverid='%d' AND port='%d'", hostid, port_num);
	ql = strlen(qry);

	if (mysql_real_query(mysqlConnection, qry, ql)) {
		printf("mysql query error\n");
		if (mysql_error(mysqlConnection)[0] != '\0') {
			printf("MySQL Said:\n%s\n", mysql_error(mysqlConnection));
		}
		close_sql(mysqlConnection);
		return -1;
	}
	if (!(res = mysql_store_result(mysqlConnection))) {
		printf("mysql store error\n");
		if (mysql_error(mysqlConnection)[0] != '\0') {
			printf("MySQL Said:\n%s\n", mysql_error(mysqlConnection));
		}
		close_sql(mysqlConnection);
		return -1;
	}

	num_rows = mysql_num_rows(res);

	if (num_rows == 0) {
		mysql_free_result(res);
		close_sql(mysqlConnection);
		/* if there is no record (new host) we'll assume it's up */
		return 0;
	}

	if ( !(row = mysql_fetch_row(res)) ) {
		mysql_free_result(res);
		close_sql(mysqlConnection);
		return -1;
	}

	current_status = atoi(row[0]);

	mysql_free_result(res);
	close_sql(mysqlConnection);

	return current_status;	
}
