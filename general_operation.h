/**
 * File: general_operation.h
 * Description: some IO and socket function wrappers, some error functions,
 *              learnt from APUE and UNP
 * Author: Burgess Wong
 * Created Date: 2013-9-28
 */

#ifndef GENERAL_OPERATION_H_
#define GENERAL_OPERATION_H_

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 4096
#define MEDIUMLINE 256
#define MINILINE 16

/* basic IO wrapper functions */

int Open(const char *pathname, int oflag);
void Close(int filedes);
ssize_t Read(int filedes, void *buf, size_t nbytes);
ssize_t Write(int filedes, const void *buf, size_t nbytes);

/* socket wrapper functions */

int Socket(int family, int type, int protocol);
void Connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);
void Bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
void Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);

ssize_t Recvfrom(int sockfd, void *buf, size_t nbytes, int flags, struct sockaddr *from, socklen_t *addrlen);
ssize_t Sendto(int sockfd, const void *buf, size_t nbytes, int flags, const struct sockaddr *to, socklen_t addrlen);

int Inet_pton(int family, const char *strptr, struct in_addr *addrptr);
const char *Inet_ntop(int family, const void *addrptr, char *strptr, size_t len);

/* error functions */

void err_sys(const char *, ...);

#endif /* GENERAL_OPERATION_ */
