/**
 * File: udppinger_server.c
 * Description: cn-atda6 socket programming assignment 2, c version
 * Author: Burgess Wong
 * Created Date: 2013-10-11
 */

#include "general_operation.h"

#define MSGLEN MAXLINE

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddrlen;

    /* initialize the socket */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);

    Bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    fputs("---Ready for receiving and responding ping messages\n", stdout);

    char buf[MSGLEN], cliaddrip[INET_ADDRSTRLEN];
    int n;
    
    /* infinite loop waiting for ping messages */
    while (1) {
	cliaddrlen = sizeof(cliaddr);
	n = Recvfrom(sockfd, buf, MSGLEN, 0, (struct sockaddr *)&cliaddr, &cliaddrlen);

	buf[n] = '\0';
	Inet_ntop(AF_INET, &cliaddr.sin_addr, cliaddrip, INET_ADDRSTRLEN);

	fprintf(stdout, "---Receive a ping from %s:%d\n", cliaddrip, ntohs(cliaddr.sin_port));
	
	buf[1] = 'o';
	
	Sendto(sockfd, buf, n, 0, (const struct sockaddr *)&cliaddr, cliaddrlen);
    }
    
    return 0;
}


