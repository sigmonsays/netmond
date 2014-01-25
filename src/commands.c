#include "common.h"

#include "config.h"
#include "commands.h"

#include "logging.h"
#include "sqlSupport.h"

#include "smtp-test.h"
#include "pop3-test.h"
#include "imap-test.h"
#include "rad-test.h"


extern int doFork;
extern int logging;
extern struct mconfig *main_config;
extern struct cfg *tcfg;

int clientRequest(int client, struct sockaddr_in cl_addr, int clinum) {

	MYSQL *mysqlConnection;
	struct sql_data *sql_res;
	struct host_ent *hostInfo;

	int r, cmdUnknown, i, j;
	char cmdline[1024], buffer[1024], cmd[64], args[1024], buf[1024];
	char *p;
	char logMessage[1024];

	char *tmp, *arg1, *arg2;

	mysqlConnection = init_sql();
	if (!mysqlConnection) {
		return -1;
	}

	memset(&buffer, 0, sizeof(buffer));
	r = recv(client, buffer, sizeof(buffer) - 1, 0);


	if (r > 0) {

		cmdUnknown = 0;
		memset(&cmdline, 0, 1024);
		memset(&cmd, 0, 64);
		memset(&args, 0, 1024);

		strncpy(cmdline, buffer, r);
		cmdline[r] = '\0';

		/* strip off cr and lf*/
		p = cmdline;
		i = 0;
		j = 0;
		while (*p++) {
			i++;
			if (strncmp(p, "\r", 1) == 0) {
				j = i;
				break;
			}
			if (strncmp(p, "\n", 1) == 0) {
				j = i;
				break;
			}
		}
		cmdline[j] = '\0';

		if (strlen(cmdline) == 0) {
			close_sql(mysqlConnection);
			return -1;
		}

		sprintf(logMessage, "cmdline = '%s' %d bytes\n", cmdline, strlen(cmdline));
		_log(logMessage, 1);

		if (strncmp(cmdline, "hosts", 5) == 0) {

			hostInfo = (void *) malloc(sizeof(struct host_ent));
			sql_res = (void *) malloc(sizeof(struct sql_data));


			if (mysqlConnection) {
				r = enumerate_hosts(mysqlConnection, NULL, sql_res);

				if (r == 0) {
					while (enumerate_hosts(mysqlConnection, hostInfo, sql_res) == 0) {
						sprintf(buf, "%d %s (%s)", hostInfo->id, hostInfo->hostname, hostInfo->ports);

						free(hostInfo->hostname);
						hostInfo->hostname = NULL;

						free(hostInfo->ports);
						hostInfo->ports = NULL;

						clientSend(client, buf);
					}

				} else {
					if (r == 1)
						clientSend(client, "no records");
					else if (r == 2)
						clientSend(client, "query failed");
					else
						clientSend(client, "unknown error");
				}

			} else {
				clientSend(client, "database error");
			}

			free(hostInfo);
			free(sql_res);

		} else if (strncmp(cmdline, "help", 4) == 0) {
			clientSend(client, 	"hosts				list hosts\n"
						"reconfig			reload config\n"
						"rad host secret		test radius\n"
						"imap host			test imap\n"
						"pop3 host			test pop3\n"
						"smtp host			test smtp\n"
						"help				this command\n"
						"notify [0|1]			disabled/enable notifications\n"
						"status				display status\n"
						"interval			set check interval (seconds)\n"
				);


		} else if (strncmp(cmdline, "rad", 3) == 0) {
			arg1 = arg2 = NULL;
			tmp = strtok(cmdline, " ");
			if (tmp) {
				if ((tmp = strtok(NULL, " "))) {
					arg1 = tmp;
				} else {
					clientSend(client, "invalid host argument");
				}

				if ((tmp = strtok(NULL, " "))) {
					arg2 = tmp;
				} else {
					clientSend(client, "invalid secret argument");
				}
			}

			if (arg1 && arg2) {
				r = check_rad(arg1, arg2);
				if (r == 0) {
					clientSend(client, "up");
				} else if (r == 1) {
					clientSend(client, "down");
				} else if (r == -1) {
					clientSend(client, "error");
				}
			}


		} else if (strncmp(cmdline, "pop3", 4) == 0) {
			arg1 = NULL;
			tmp = strtok(cmdline, " ");
			if (tmp) {
				if ((tmp = strtok(NULL, " "))) {
					arg1 = tmp;
				} else {
					clientSend(client, "invalid argument");
				}
			}

			if (arg1) {
				r = check_pop3(arg1);
				if (r == 0) {
					clientSend(client, "up");
				} else if (r == 1) {
					clientSend(client, "down");
				} else if (r == -1) {
					clientSend(client, "error");
				}
			}


		} else if (strncmp(cmdline, "imap", 4) == 0) {
			arg1 = NULL;
			tmp = strtok(cmdline, " ");
			if (tmp) {
				if ((tmp = strtok(NULL, " "))) {
					arg1 = tmp;
				} else {
					clientSend(client, "invalid argument");
				}
			}

			if (arg1) {
				r = check_imap(arg1);
				if (r == 0) {
					clientSend(client, "up");
				} else if (r == 1) {
					clientSend(client, "down");
				} else if (r == -1) {
					clientSend(client, "error");
				}
			}

		} else if (strncmp(cmdline, "smtp", 4) == 0) {
			arg1 = NULL;
			tmp = strtok(cmdline, " ");
			if (tmp) {
				if ((tmp = strtok(NULL, " "))) {
					arg1 = tmp;
				} else {
					clientSend(client, "invalid argument");
				}
			}

			if (arg1) {
				r = check_smtp(arg1);
				if (r == 0) {
					clientSend(client, "up");
				} else if (r == 1) {
					clientSend(client, "down");
				} else if (r == -1) {
					clientSend(client, "error");
				}
			}


		} else if (strncmp(cmdline, "reconfig", 8) == 0) {
			clientSend(client, "Invoking reload");
			_reload();
			clientSend(client, "reload ok");

		} else if (strncmp(cmdline, "notify", 6) == 0) {
			arg1 = NULL;
			tmp = strtok(cmdline, " ");
			if (tmp) {
				if ((tmp = strtok(NULL, " "))) {
					arg1 = tmp;
				} else {
					clientSend(client, "invalid argument");
				}
			}

			if (arg1) {
				main_config->notifications = atoi(arg1);
				if (main_config->notifications) {
					clientSend(client, "Notifications enabled");
				} else {
					clientSend(client, "Notifications disabled");
				}
			}

		} else if (strncmp(cmdline, "interval", 8) == 0) {
			arg1 = NULL;
			tmp = strtok(cmdline, " ");
			if (tmp) {
				if ((tmp = strtok(NULL, " "))) {
					arg1 = tmp;
				} else {
					clientSend(client, "invalid argument");
				}
			}

			if (arg1) {
				main_config->interval = atoi(arg1);
				alarm(main_config->interval);

				clientSend(client, "interval set");
			}

		} else if (strncmp(cmdline, "status", 6) == 0) {

			if (main_config->last_check != -1) {
				sprintf(buf, "last check %s", ctime(&main_config->last_check) );
				clientSend(client, buf);
			}

			sprintf(buf, "interval %d sec", main_config->interval);
			clientSend(client, buf);

			sprintf(buf, "notify %d", main_config->notifications);
			clientSend(client, buf);
			
		}

		close_sql(mysqlConnection);

		/* this signals the main loop to close our connection -- we wanna disconnect after each request */
		return -1;

	}

	return (r == -1) ? 0 : r;
}
