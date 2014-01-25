#include "common.h"
#include "rad-test.h"
#include "config.h"

extern struct mconfig *main_config;

/*
        function to check radius using the radclient program (from cistron radius)
        returns 0 on sucess, 1 on closed, -1 on error
*/

int check_rad(char *host, char *secret) {
	char buf[1024], rbuf[1024];
	int fdin[2], fdout[2];
	int childrc, pid, nbytes;
	char *tmp;
	int r, rv = 1;

	if (pipe(fdin) != 0) return -1;
	if (pipe(fdout) != 0) return -1;

	pid = fork();
	if (pid == 0) { /* child process */
		close(fdin[1]);
		dup2(fdin[0], 0);

		close(fdout[0]);
		dup2(fdout[1], 1);

		close(2);

		r = execlp("radclient", "radclient", host, "auth", secret, NULL);
		if (r == -1) { /* binary not found! */
			write(fdout[1], "radclient_error\n", 16);
		}
		exit(0);

	} else if (pid < 0) { /* forkin' fork() error */
		return -1;
	}


	if (pid) {	/* parent process */

		close(fdin[0]);
		close(fdout[1]);

		sprintf(buf, "User-Name = \"%s\", Password = \"%s\"\r\n", main_config->rad->user, main_config->rad->pass);
		write(fdin[1], buf, strlen(buf));
		close(fdin[1]);

		/* read response (stdout) */
		nbytes = read(fdout[0], rbuf, sizeof(rbuf));
		if (nbytes > 0) {
			rbuf[nbytes] = '\0';
			tmp = strtok(rbuf, "\n");
			if (tmp) {
				while (1) {
					if (strncmp("    Service-Type", tmp, 16) == 0) {
						rv = 0;
						break;
					}

					if (!(tmp = strtok(NULL, "\n"))) break;
				}
			}
		}

		wait(&childrc);
	}

	close(fdout[0]);
	return rv;
}
