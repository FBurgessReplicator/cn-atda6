/**
 * File: udppinger_client.c
 * Description: cn-atda6 socket programming assignment 2, c version
 * Author: Burgess Wong
 * Created Date: 2013-10-11
 */

#include "general_operation.h"

#define MSGLEN 6

bool checkresp(int sockfd, int i) {
    char buf[MSGLEN + 1], num[2];

    buf[6] = '\0';
    
    Recvfrom(sockfd, buf, MSGLEN, 0, NULL, NULL);
    memcpy((char *)num, (char *)(buf + 4), 2);

    return atoi(num) == i ? true : false;
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2)
	err_quit("usage: udppinger_client <IPaddress>\n");
    
    /* initialize the socket */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    char pingbuf[MSGLEN];
    strcpy(pingbuf, "ping0");

    clock_t firsttime, starttime, endtime;
    fd_set rset;
    int i, n;
    struct timeval resttime;

    /* send 10 ping messages */
    for (i = 0; i < 10; ++i) {
	pingbuf[5] = (char)(i + 48);
	
	Sendto(sockfd, pingbuf, MSGLEN, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

	resttime.tv_sec = 0;
	resttime.tv_usec = 1000000;
	firsttime = clock();
	
	while (resttime.tv_usec > 0) {
	    FD_ZERO(&rset);
	    FD_SET(sockfd, &rset);
	    
	    starttime = clock();
	    n = Select(sockfd + 1, &rset, NULL, NULL, &resttime);
	    endtime = clock();

	    if (n == 0) { /* sockfd not available, timeout */
		fprintf(stdout, "---ping %.2d timeout\n", i);
		break;
	    } else { /* receive a pong message, check it */
		resttime.tv_usec -= (endtime - starttime) / CLOCKS_PER_SEC * 1000000;

		if (checkresp(sockfd, i)) { /* a corresponding message, print OK */
		    fprintf(stdout, "---pong %.2d received, RTT=%.3lfms\n", i, (double)(endtime - firsttime) / \
			    CLOCKS_PER_SEC * 1000);
		    select(0, NULL, NULL, NULL, &resttime);
		    break;
		}
	    }
	}
	
    }
    
    return 0;
}


