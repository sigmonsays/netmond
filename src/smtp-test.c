#include "common.h"
#include "smtp-test.h"
#include "net-io.h"
#include "config.h"
#include "netmond.h"

extern struct mconfig *main_config;

/*
	function to login and send mail via smtp
	returns 0 on sucess, 1 on closed, -1 on error
*/

int check_smtp(char *host) {
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
	dest_addr.sin_port = htons(25);

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

	/* send helo */
	sprintf(buf, "HELO %s\r\n", host);
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	free(tmp);

	/* send mail from */
	sprintf(buf, "MAIL FROM:<%s@%s>\r\n", main_config->smtp->user, host);
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	free(tmp);

	/* send rcpt */
	sprintf(buf, "RCPT TO:<%s@%s>\r\n", main_config->smtp->user, host);
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	free(tmp);

	/* send data */
	sprintf(buf, "DATA\r\n");
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	free(tmp);

	/* send message */
	sprintf(buf,
		"To: <%s>\r\nFrom: <%s>\r\nSubject: smtp test message\r\n\r\nThis is a test\r\n.\r\n",
			main_config->smtp->user, main_config->smtp->user);

	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}
	free(tmp);

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
