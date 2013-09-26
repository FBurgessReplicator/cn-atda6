/**
 * File: webserver.c
 * Description: cn-atda6 socket programming assignment 1, c version
 * Author: Burgess Wong
 * Created Date: 2013-9-26
 * Notice: compile with -std=c99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXBUF 4096
#define LITTLEBUF 256

void resolve_request(const char *reqbuf, char *urlbuf, char *versionbuf) {
    char *pos, *pos2;

    // copy url
    pos = strchr(reqbuf, (int)' ');
    pos2 = strchr(pos + 1, (int)' ');
    strncpy(urlbuf, pos + 1, pos2 - pos);
    urlbuf[pos2 - pos] = '\0';
    
    // copy version
    pos = strchr(pos2 + 1, (int)'\r');
    strncpy(versionbuf, pos2 + 1, pos - pos2);
    versionbuf[pos - pos2] = '\0';
}

int main(int argc, char *argv[]) {
    int listenfd, connfd;
    struct sockaddr_in cliaddr, servaddr;

    // initialize the socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0) < 0)) {
	fputs("socket error", stderr);
	exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);

    if (bind(listenfd, (struct sockaddr *)&seraddr, sizeof(servaddr)) < 0) {
	fputs("bind error");
	exit(1);
    }

    if (listen(listenfd, 1) < 0) {
	fputs("listen error");
	exit(1);
    }

    char *reqbuf, *respbuf, urlbuf[LITTLEBUF + 1], versionbuf[LITTLEBUF + 1];
    reqbuf = (char *)malloc((MAXBUF + 1) * sizeof(char));
    respbuf = (char *)malloc((MAXBUF + 1) * sizeof(char));

    // infinite loop waiting for and processing requests
    int n;
    while (1) {
	fputs("---Listening", stdout);
	fputs("", stdout);
	
	if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr))) < 0) {
	    fputs("accept error", stderr);
	    exit(1);
	}

	if ((n = read(connfd, reqbuf, MAXBUF)) < 0) {
	    fputs("read error", stderr);
	    exit(1);
	}

	if (n == 0) {
	    close(connfd);
	    continue;
	}

	reqbuf[n] = '\0';
	
	// resolve the request
	resolve_request(reqbuf, (char *)urlbuf, (char *)versionbuf);

	fprintf(stdout, "%s%s", "---Requested file path: ", urlbuf);

	// try to open the file in the file system
	int fd;
	char *status_code, *phrase;
	const char *notfound_str = "404 File Not Found - by Burgess Wong";
	bool readok;
	if ((fd = open(urlbuf, O_RDONLY)) < 0) {

	    status_code = "404";
	    phrase = "Not Found";
	    readok = false;
	} else {
	    status_code = "200";
	    phrase = "OK";
	    
	    readok = true;
	}

	sprintf(respbuf, "%s %s %s\r\n\r\n", versionbuf, status_code, phrase);
	if (write(connfd, respbuf, strlen(respbuf)) != strlen(respbuf)) {
	    fputs("write socket error", stderr);
	    exit(1);
	}

	if (readok) {
	    while ((n = read(connfd, respbuf, MAXBUF)) > 0)
		if (write(connfd, respbuf, n) != n) {
		    fputs("write error", stderr);
		    exit(1);
		}
	} else {
	    if (write(connfd, notfound_str, strlen(notfound_str)) != strlen(notfound_str)) {
		fputs("write error", stderr);
		exit(1);
	    }
	}

	fclose(fd);
	fclose(connfd);
    }

    return 0;
}
	






		    
