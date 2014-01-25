#include "common.h"
#include "netmond.h"
#include "netcheck.h"
#include "config.h"
#include "sqlSupport.h"
#include "check-ips.h"
#include "logging.h"

int debug = 0;
int doFork = 1;
int logging = 0;

char logMessage[1024];
char *logfile = NULL;
char *cfg_file = NULL;

int netcheck_running = 0;

int clients[MAX_CLIENTS];

struct mconfig *main_config;

int main(int argc, char *argv[]) {

	int notifications = -1;

	int sdown = 0;

	int client, cl_addrlen, my_addrlen, r;

	struct sockaddr_in my_addr;
	struct sockaddr_in cl_addr;
	int pid, yes = 1, i, clinum, new_client, sock;

	int max_fd;
	fd_set fds;
	struct timeval timeout;

	int port = PORT;

	char tmp[1024];

	signal(SIGALRM, (void *) check_servers);
	signal(SIGTERM, (void *) _quit);
	signal(SIGINT, (void *) _quit);
	signal(SIGHUP, (void *) _reload);
	

	for(r=0; r<MAX_CLIENTS; r++) {
		clients[r] = -1;
	}

	memset(&logMessage, 0, sizeof(logMessage));

	cfg_file = NULL;
	
	/* parse cmd line options */
	for(i=0; i<argc; i++) {

		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
			debug = 1;

		} else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--config")) {
			if (argv[i + 1]) {
				cfg_file = strdup(argv[i + 1]);
			} else {
				printf("config file needed!");
				exit(1);
			}

		} else if (!strcmp(argv[i], "-fg") || !strcmp(argv[i], "--foreground")) {
			doFork = 0;

		} else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")) {
			if (argv[i + 1]) {
				port = atoi(argv[i + 1]);
			} else {
				printf("port parameter required\n");
				exit(1);
			}

		} else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--logfile")) {
			logging = 1;
			if (argv[i + 1]) {
				logfile = strdup(argv[i + 1]);
			} else {
				printf("logfile parameter required\n");
				exit(1);
			}

		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			_help();
			exit(0);

		} else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
			_help();
			exit(0);

		} else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--no-notifications")) {
			notifications = 0;
		}
	}

	if (!cfg_file) cfg_file = strdup("/etc/netmond.cfg");

	/* parse config */
	if (!(main_config = parse_cfg(cfg_file))) {
		printf("Can't find config file: %s\n", cfg_file);
		exit(1);
	}

	if (notifications != -1)
		main_config->notifications = 0;


	if (main_config->database == 0) {
		_log("Failed to connect to mysql server!\n", 0);
	}

	memset(&my_addr, 0, sizeof(my_addr));
	memset(&cl_addr, 0, sizeof(cl_addr));

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = BIND_ADDR;          /* Broadcast address */
	my_addr.sin_port = htons(port);

	cl_addrlen = sizeof(cl_addr);
	my_addrlen = sizeof(my_addr);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
                perror ("error setting SO_REUSEADDR on listening socket");
                return -1;
        }


	if (bind(sock, (struct sockaddr*) &my_addr, my_addrlen) == -1) { /* Must have this on server */
		perror("bind");
		exit(1);
	}

	if (listen(sock, 5) == -1) {
		perror("listen");
		exit(1);
	}

	fcntl(sock, F_SETFL, O_NONBLOCK);

	if (!logging) printf("Warning: Logging is disabled!\n");


	if (doFork) {
		printf("forking into background...\n");

		pid = fork();
		if (pid == -1) {
			perror("fork");
			exit(1);
		}
		if (pid) {
			exit(0);
		}
	}

	alarm(main_config->interval);

	_log("-- MARK --\n", 0);
	sprintf(logMessage, "netmond started on port %d with pid %d\n", port, getpid());
	_log(logMessage, 0);

	/* enter main loop */
	while (1) {
		new_client = accept(sock, (struct sockaddr *)&cl_addr, &cl_addrlen);

		/* we have a new client */
		if (new_client > 0) {
			fcntl(new_client, F_SETFL, O_NONBLOCK);

			if (check_ip(main_config, inet_ntoa(cl_addr.sin_addr)) == 0) {

				sprintf(logMessage, "%s : connection closed -- not allowed in.\n", inet_ntoa(cl_addr.sin_addr));
				_log(logMessage, 0);

				shutdown(new_client, 2);
				close(new_client);

			} else {
				clinum = -1;
				for(i=0; i<MAX_CLIENTS; i++) {
					if (clients[i] == -1) {
						clients[i] = new_client;
						clinum = i;
						break;
					}
				}

				if (clinum == -1) {
					clientSend(new_client, "Sorry. No more connections allowed!");
					shutdown(new_client, 2);
					close(new_client);
				}

				sprintf(logMessage, "%s (%d) : connected.\n", inet_ntoa(cl_addr.sin_addr), clinum);
				_log(logMessage, 0);
			}
		}


		/* deal with connected clients */
                timeout.tv_sec = 0;
                timeout.tv_usec = 100;
                FD_ZERO(&fds);
		max_fd = 0;
		for(i=0; i<MAX_CLIENTS; i++) {
			if (clients[i] > 0) {
				FD_SET(clients[i], &fds);
				if (clients[i] > max_fd) max_fd = clients[i];
			}
		}

		r = select(max_fd + 1, &fds, NULL, NULL, &timeout);

		for(i=0; i<MAX_CLIENTS; i++) {
			if (clients[i] > 0) {
				if (FD_ISSET(clients[i], &fds)) {
					client = clients[i];

					sdown = clientRequest(client, cl_addr, i);

					if (sdown == -1) { /* close connection */
						shutdown(client, 2);
						close(client);

						clients[i] = -1;

						sprintf(logMessage, "%s (%d) : disconnected.\n", (char *)inet_ntoa(cl_addr.sin_addr), i);
						_log(logMessage, 0);

					}
				}
			}
		}
		FD_ZERO(&fds);

		/* destroy old connections */
		for(i=0; i<MAX_CLIENTS; i++) {
			client = clients[i];
			if (client > 0) {
				r = read(client, &tmp, sizeof(tmp));
				if (r == 0) {
					/* this is a dead connection */
					shutdown(client, SHUT_RDWR);
					close(client);
					clients[i] = -1;

					sprintf(logMessage, "%s (%d) : dead connection, closing..\n", inet_ntoa(cl_addr.sin_addr), i);
					_log(logMessage, 0);
				}
			}
		}

	} /* main loop */


	/* I should never reach this */
	return 0;
}

void _quit() {
	int i, client;

	sprintf(logMessage, "shutting down!\n");
	_log(logMessage, 0);

	for(i=0; i<MAX_CLIENTS; i++) {
		client = clients[i];
		if (client > 0) {
			shutdown(client, SHUT_RDWR);
			close(client);
		}
	}

	if (cfg_file) free(cfg_file);
	free_config(main_config);
	exit(0);
}

void _help(void) {
	printf(	"netmond %d.%d.%d\n"
		"-h --help             Print this help message\n"
		"-c --config           Config file to use\n"
		"-a --allow            Allow only this IP to connect\n"
		"-fg --foreground      Don't fork\n"
		"-d --debug            Log debugging information\n"
		"-p --port port        Specify port, default 2421\n"
		"-l --logfile file     Log to file default off\n"
		"-v --version          display version\n",

		VER_MAJOR, VER_MINOR, VER_MICRO
	);
}

void _reload(void) {

	sprintf(logMessage, "Caught HUP signal, reloading configuration...\n");
	_log(logMessage, 0);

	free_config(main_config);

	main_config = parse_cfg(cfg_file);

	if (!main_config) {
		printf("Critical error reparsing config file (%s), exiting now...\n", cfg_file);
		_quit();
	}

	alarm(main_config->interval);

	sprintf(logMessage, "configuration reload complete\n");
	_log(logMessage, 0);
}

int clientSend(int c, char *msg) {
        int r;
        char buf[strlen(msg) + 3];
        strcpy(buf, msg);
        strcat(buf, "\r\n");
        r = send(c, buf, strlen(buf), 0);
        fflush(NULL);
        return r;
}

