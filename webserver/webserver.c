/**
 * File: webserver.c
 * Description: cn-atda6 socket programming assignment 1, c version
 * Author: Burgess Wong
 * Created Date: 2013-9-26
 */

#include "general_operation.h"

void resolve_request(const char *reqbuf, char *urlbuf, char *versionbuf) {
    char *pos, *pos2;

    /* copy url */
    pos = strchr(reqbuf, (int)' ');
    pos2 = strchr(pos + 1, (int)' ');
    strncpy(urlbuf, pos + 1, pos2 - pos - 1);
    urlbuf[pos2 - pos - 1] = '\0';
    
    /* copy version */
    pos = strchr(pos2 + 1, (int)'\r');
    strncpy(versionbuf, pos2 + 1, pos - pos2 - 1);
    versionbuf[pos - pos2 - 1] = '\0';
}

int main(int argc, char *argv[]) {
    int listenfd, connfd;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t cliaddrlen;

    /* initialize the socket */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Listen(listenfd, 1);

    char *reqbuf, *respbuf, urlbuf[MEDIUMLINE + 1], versionbuf[MINILINE + 1], cliaddrip[INET_ADDRSTRLEN];
    reqbuf = (char *)malloc((MAXLINE + 1) * sizeof(char));
    respbuf = (char *)malloc((MAXLINE + 1) * sizeof(char));

    /* infinite loop waiting for and processing requests */
    int n;
    while (1) {
	fputs("---Listening\n", stdout);

	cliaddrlen = sizeof(cliaddr);
	connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddrlen);

	Inet_ntop(AF_INET, &cliaddr.sin_addr, cliaddrip, INET_ADDRSTRLEN);

	fprintf(stdout, "%s%s\n", "---Request comes from IP: ", cliaddrip);
	
	n = Read(connfd, reqbuf, MAXLINE);

	if (n == 0) {
	    Close(connfd);
	    continue;
	}

	reqbuf[n] = '\0';
	
	/* resolve the request */
	resolve_request(reqbuf, (char *)urlbuf, (char *)versionbuf);
	
	fprintf(stdout, "---Print url and version %s sep %s\n", urlbuf, versionbuf);
	
	fprintf(stdout, "%s%s\n", "---Requested file path: ", urlbuf);

	/* try to open the file in the file system */
	int fd;
	char *status_code, *phrase;
	const char *notfound_str = "404 File Not Found - by Burgess Wong";
	bool readok;
	
	if ((fd = open(urlbuf, O_RDONLY)) < 0) {
	    status_code = "404";
	    phrase = "Not Found";
	    readok = false;
	    fprintf(stderr, "%s%s%s\n", "---File ", urlbuf, " not found, generate 404 Not Found response");
	} else {
	    status_code = "200";
	    phrase = "OK";
	    readok = true;
	    fprintf(stderr, "%s%s%s\n", "---File ", urlbuf, " found, generate 200 Found response");
	}

	sprintf(respbuf, "%s %s %s\r\nConnection: closed\r\n\r\n", versionbuf, status_code, phrase);

	Write(connfd, respbuf, strlen(respbuf));
	
	if (readok) {
	    while ((n = Read(fd, respbuf, MAXLINE)) > 0)
		Write(connfd, respbuf, n);
	} else {
	    Write(connfd, notfound_str, strlen(notfound_str));
	}

	fputs("---Response sent\n", stdout);

	if (readok)
	    Close(fd);
	Close(connfd);

	fprintf(stdout, "%s%s%s\n", "---Connection with IP ", cliaddrip, " closed");
    }

    return 0;
}
	
