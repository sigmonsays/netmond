#include "common.h"
#include "imap-test.h"
#include "net-io.h"
#include "config.h"
#include "netmond.h"

extern struct mconfig *main_config;

/*
	function to login and check mail via imap protocol
	returns 0 on sucess, 1 on closed (failure), -1 system error

	NOTE:
		If it gets an invalid login there is a time-out on the courier server, longer than
		the one configured in net-io.c (4 seconds). It will say the service is down.
*/

int check_imap(char *host) {
	int sockfd, s;
	struct sockaddr_in dest_addr;
#ifdef USE_GETHOSTBYNAME
	struct hostent *hp;
#endif
	char buf[1024];
	char *tmp;

	memset(&dest_addr, 0, sizeof(struct sockaddr_in));

#ifdef USE_GETHOSTBYNAME
	if ((hp = gethostbyname(host)) == NULL) {
		return -1;
	}
#endif

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(143);

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
	if (s == -1) return 1;
	if (s > 0) {
		buf[s] = '\0';
	}

	/* send user & pass */
	sprintf(buf, "A00001 LOGIN %s %s\r\n", main_config->imap->user, main_config->imap->pass);

	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 0;
	}

	if (strncmp("A00001 OK", tmp, 9) != 0) {
		/*
			login failed, but service still up... 
		*/
		shutdown(sockfd, 2);
		close(sockfd);
		free(tmp);
		return 0;
	}
	free(tmp);

	/* send list */
	strcpy(buf, "A00002 LIST \"\" \"%%\"\r\n");
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		free(tmp);
		return 0;
	}
	free(tmp);

	shutdown(sockfd, 2);
	close(sockfd);
	return 0;
}
