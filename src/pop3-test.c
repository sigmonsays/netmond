#include "common.h"
#include "pop3-test.h"
#include "net-io.h"
#include "config.h"
#include "netmond.h"

extern struct mconfig *main_config;

/*
	function to login and check mail via pop3 protocol
	returns 0 on sucess, 1 on closed (failure), -1 system error
*/

int check_pop3(char *host) {
	int sockfd, s;
	struct sockaddr_in dest_addr;
#ifdef USE_GETHOSTBYNAME
	struct hostent *hp;
#endif
	char buf[1024];
	char *tmp = NULL;
	int msg_count, box_size;

	memset(&dest_addr, 0, sizeof(struct sockaddr_in));

#ifdef USE_GETHOSTBYNAME
        if ((hp = gethostbyname(host)) == NULL) {
                return -1;
        }
#endif
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(110);

#ifdef USE_GETHOSTBYNAME
        dest_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
#else
        dest_addr.sin_addr.s_addr = inet_addr(host);
#endif

	/* this will usually return -1 on a non-blocking socket */
	connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));



	/* wait for banner message */
	if (wait_io(1, sockfd, 4, 0) == -1) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	s = read(sockfd, &buf, sizeof(buf));
	if (s == -1) {
		return 1;
	}
	if (s > 0) {
		buf[s] = '\0';
	}

	/* send user */
	sprintf(buf, "USER %s\r\n", main_config->pop3->user);
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}

	if (strncmp("+OK", tmp, 3) != 0) {
		shutdown(sockfd, 2);
		close(sockfd);
		free(tmp);
		return 0;
	}
	free(tmp);

	/* send pass */
	sprintf(buf, "PASS %s\r\n", main_config->pop3->pass);
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	if (strncmp("+OK", tmp, 3) != 0) {
		/* invalid password right here, we'll still say
		   the pop3 is good =) --- good pop3
		*/
		shutdown(sockfd, 2);
		close(sockfd);
		free(tmp);
		return 0;
	}
	free(tmp);

	/* send stat */

	sprintf(buf, "STAT\r\n");
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}

	s = sscanf(tmp, "%*s %d %d", &msg_count, &box_size);
	if (s == 2) {
		free(tmp);
		if (msg_count > 0) {
			/* delete a message */
			sprintf(buf, "DELE 1\r\n");
			if (!(tmp = net_request(sockfd, buf))) {
				shutdown(sockfd, 2);
				close(sockfd);
				return 1;
			}
			free(tmp);
		}

	} else {
		free(tmp);
	}
	

	/* send quit */
	sprintf(buf, "QUIT\r\n");
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	free(tmp);

	shutdown(sockfd, 2);
	close(sockfd);
	return 0;
}
