/**
 * File: general_operation.h
 * Description: impleations of general_operation.h
 * Author: Burgess Wong
 * Created Date: 2013-9-28
 */

#include "general_operation.h"

/* basic IO wrapper functions */

int Open(const char *pathname, int oflag) {
    int filedes;
    if ((filedes = open(pathname, oflag)) < 0)
	err_sys("open error");

    return filedes;
}

void Close(int filedes) {
    if (close(filedes) < 0)
	err_sys("close error");
}

ssize_t Read(int filedes, void *buf, size_t nbytes) {
    int n;
    if ((n = read(filedes, buf, nbytes)) < 0)
	err_sys("read error");

    return n;
}

ssize_t Write(int filedes, const void *buf, size_t nbytes) {
    if (write(filedes, buf, nbytes) != nbytes)
	err_sys("write error");

    return nbytes;
}

/* socket wrapper functions */

int Socket(int family, int type, int protocol) {
    int filedes;
    if ((filedes = socket(family, type, protocol)) < 0)
	err_sys("socket error");

    return filedes;
}

void Connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen) {
    if (connect(sockfd, servaddr, addrlen) < 0)
	err_sys("listen error");
}

void Bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen) {
    if (bind(sockfd, myaddr, addrlen) < 0)
	err_sys("bind error");
}

void Listen(int sockfd, int backlog) {
    if(listen(sockfd, backlog) < 0)
	err_sys("listen error");
}

int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen) {
    int clisockfd;
    if ((clisockfd = accept(sockfd, cliaddr, addrlen)) < 0)
	err_sys("accept error");
    
    return clisockfd;
}

ssize_t Recvfrom(int sockfd, void *buf, size_t nbytes, int flags, struct sockaddr *from, socklen_t *addrlen) {
    int n;
    if ((n = recvfrom(sockfd, buf, nbytes, flags, from, addrlen)) < 0)
	err_sys("recvfrom error");

    return n;
}

ssize_t Sendto(int sockfd, const void *buf, size_t nbytes, int flags, const struct sockaddr *to, socklen_t addrlen) {
    if (sendto(sockfd, buf, nbytes, flags, to, addrlen) != nbytes)
	err_sys("sendto error");

    return nbytes;
}

int Inet_pton(int family, const char *strptr, struct in_addr *addrptr) {
    if (inet_pton(family, strptr, addrptr) != 1)
	err_sys("inet_pton error");

    return 1;
}

const char *Inet_ntop(int family, const void *addrptr, char *strptr, size_t len) {
    if (inet_ntop(family, addrptr, strptr, len) == NULL)
	err_sys("inet_ntop error");

    return strptr;
}

/* error functions */

static void err_doit(int, int, const char *, va_list);

void err_sys(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    exit(1);
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
    char buf[MAXLINE];
    int n;

    snprintf(buf, MAXLINE - 1, fmt, ap);
    n = strlen(buf);
    if (errnoflag)
	snprintf(buf + n, MAXLINE - n - 1, ": %s", strerror(errno));

    strcat(buf, "\n");

    fflush(stdout);
    fputs(buf, stderr);
    fflush(stderr);
}
