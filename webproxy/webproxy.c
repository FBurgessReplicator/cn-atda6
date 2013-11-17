/**
 * File: webproxy.c
 * Description: cn-adta6 socket programming assignment 4, python version
 *              a multi-threaded web proxy
 * Author: Burgess Wong
 * Created Date: 2013-11-12
 */

#include "general_operation.h"

#define HTTP_PORT 80
#define PROXY_PORT 8080

struct header_field {
    const char *name, *value;
    int nlen, vlen;
    struct header_field *next;
};

inline int min(int i, int j) {
    return i < j ? i : j;
}

inline int max(int i, int j){
    return i > j ? i : j;
}

void get_start_fields(const char *start_line, int sllen, const char **first_field, int *flen, \
		      const char **second_field, int *slen, const char **third_field, int *tlen) {
    *first_field = start_line;
    *second_field = strstr(*first_field, " ");
    *second_field += 1;
    *flen = *second_field - *first_field - 1;
    *third_field = strstr(*second_field, " ");
    *third_field += 1;
    *slen = *third_field - *second_field - 1;
    *tlen = sllen - *flen - *slen - 2;
}

struct header_field *get_header_fields(const char *header_lines, int hlen) {
    int len;
    const char *iter_line, *rest_lines;
    struct header_field header_fields, *header_fields_tail, *new_field;

    len = 0;
    header_fields.next = NULL;
    header_fields_tail = &header_fields;
    rest_lines = header_lines;

    while (1) {
	iter_line = rest_lines;
	rest_lines = strstr(rest_lines, "\r\n") + 2;

	new_field = calloc(1, sizeof(struct header_field));
	new_field->name = iter_line;
	new_field->value = strstr(iter_line, ": ") + 2;
	new_field->nlen = new_field->value - new_field->name - 2;
	new_field->vlen = rest_lines - new_field->value - 2;
	new_field->next = NULL;

	header_fields_tail->next = new_field;
	header_fields_tail = new_field;

	len += rest_lines - iter_line;

	if (len == hlen)
	    break;
    }

    return header_fields.next;
}

void get_host_from_url(const char *url, const char **host, int *hostlen) {
    const char *rest;
    *host = strstr(url, "://");
    *host += 3;
    rest = strstr(*host, "/");
    *hostlen = rest - *host;
}

const struct header_field *get_header_field(const struct header_field *header_fields, const char *name) {
    const struct header_field *iter_field;

    for (iter_field = header_fields; iter_field; iter_field = iter_field->next)
	if (strncmp(name, iter_field->name, iter_field->nlen) == 0)
	    return iter_field;

    return NULL;
}

void remove_header_field(struct header_field **p_header_fields, const char *name) {
    struct header_field **p_iter_field, *current_field;
    p_iter_field = p_header_fields;
    while (*p_iter_field) {
	current_field = *p_iter_field;
	if (strncmp(name, current_field->name, current_field->nlen) == 0) {
	    *p_iter_field = current_field->next;
	    free(current_field);
	    return;
	} else {
	    p_iter_field = &current_field->next;
	}
    }
}

void separate_http_message(const char *message, int mlen, const char **start_line, int *slen, \
                           const char **header_lines, int *hlen, const char **body, int *blen) {
    *start_line = message;
    *header_lines = strstr(*start_line, "\r\n");
    *header_lines += 2;
    *slen = *header_lines - *start_line - 2;
    *body = strstr(*header_lines, "\r\n\r\n");
    *body += 4;
    *hlen = *body - *header_lines - 2;
    *blen = mlen - *slen - *hlen - 4;
}

char *write_to_buf(char *buf, const char *content, int clen) {
    memcpy(buf, content, clen);

    return buf + clen;
}

void do_proxy(int clifd, const struct sockaddr_in *cliaddr, int pid) {
    /* get the request from the client */
    char *request;
    int request_len;
    request = calloc(MAXLINE, sizeof(char));
    request_len = Read(clifd, request, MAXLINE);

    /* get the request line, header lines and entity body */
    const char *request_line, *header_lines, *body;
    int rlen, hlen, blen;
    separate_http_message(request, request_len, &request_line, &rlen, &header_lines, &hlen, &body, &blen);
    
    /* get header fields */
    struct header_field *header_fields;
    header_fields = get_header_fields(header_lines, hlen);

    /* remove Connection and Proxy-Connection from header fields */
    remove_header_field(&header_fields, "Connection");
    remove_header_field(&header_fields, "Proxy-Connection");

    /* get the hostname of the server */
    const struct header_field *host_header_field;
    char *host;
    const char *p_host, *method, *url, *version;
    int hostlen, methodlen, urllen, versionlen;
    if ((host_header_field = get_header_field(header_fields, "Host")) == NULL) {
	get_start_fields(request_line, rlen, &method, &methodlen, &url, &urllen, &version, &versionlen);
	get_host_from_url(url, &p_host, &hostlen);
    } else {
	p_host = host_header_field->value;
	hostlen = host_header_field->vlen;
    }
    host = calloc(1, hostlen + 1);
    memcpy(host, p_host, hostlen);

    /* connect to the server */
    int servfd;
    struct sockaddr_in servaddr;
    socklen_t servaddrlen;
    struct addrinfo hints, *result;
    fprintf(stdout, "---pid=%d, Connect to the server\n", pid);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, "http", &hints, &result);
    memcpy(&servaddr, result->ai_addr, sizeof(servaddr));
    servaddrlen = result->ai_addrlen;
    freeaddrinfo(result);

    servfd = Socket(AF_INET, SOCK_STREAM, 0);
    Connect(servfd, (struct sockaddr *)&servaddr, servaddrlen);

    /* assemble the new request, assuming that the whole request is less than 4096 bytes */
    char *new_request, *bufpos;
    int new_request_len;
    struct header_field *iter_header_field;
    new_request = calloc(MAXLINE, sizeof(char));
    new_request_len = 0;
    bufpos = new_request;
    bufpos = write_to_buf(bufpos, request_line, rlen);
    bufpos = write_to_buf(bufpos, "\r\n", 2);
    new_request_len += rlen + 2;
    for (iter_header_field = header_fields; iter_header_field; iter_header_field = iter_header_field->next) {
	bufpos = write_to_buf(bufpos, iter_header_field->name, iter_header_field->nlen);
	bufpos = write_to_buf(bufpos, ": ", 2);
	bufpos = write_to_buf(bufpos, iter_header_field->value, iter_header_field->vlen);
	bufpos = write_to_buf(bufpos, "\r\n", 2);
	new_request_len += iter_header_field->nlen + iter_header_field->vlen + 4;
    }
    bufpos = write_to_buf(bufpos, "\r\n", 2);
    bufpos = write_to_buf(bufpos, body, blen);
    new_request_len += blen + 2;

    /* send the new request to the server */
    fprintf(stdout, "---pid=%d, Send the new request to the server\n", pid);
    Write(servfd, new_request, new_request_len);

    /* get and send back the response */
    char *response;
    int response_len;
    fprintf(stdout, "---pid=%d, Send the response back to the client\n", pid);
    response = calloc(MAXLINE, sizeof(char));

    while ((response_len = Read(servfd, response, MAXLINE)) > 0) {
	fprintf(stdout, "---pid=%d, %d bytes received from the server\n", pid, response_len);
	Write(clifd, response, response_len);
    }
    
    /* close the server socket */
    fprintf(stdout, "---pid=%d, Close the server socket\n", pid);
    close(servfd);

    /* release memory */
    struct header_field *next_header_field;
    free(request);
    free(new_request);
    free(response);
    free(host);
    for (iter_header_field = header_fields; iter_header_field; iter_header_field = next_header_field) {
	next_header_field = iter_header_field->next;
	free(iter_header_field);
    }
}

int main(int argc, char *argv[]) {
    int proxyfd, clifd;
    struct sockaddr_in proxyaddr, cliaddr;
    socklen_t cliaddrlen;

    /* initialize the socket as a server */
    proxyfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&proxyaddr, sizeof(proxyaddr));
    proxyaddr.sin_family = AF_INET;
    proxyaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    proxyaddr.sin_port = htons(PROXY_PORT);
    Bind(proxyfd, (struct sockaddr *)&proxyaddr, sizeof(proxyaddr));
    Listen(proxyfd, 5);

    /* infinite loop waiting for and processing requests */
    char cliaddrip[INET_ADDRSTRLEN];
    int pid;
    while (1) {
	cliaddrlen = sizeof(cliaddr);
	clifd = Accept(proxyfd, (struct sockaddr *)&cliaddr, &cliaddrlen);
	Inet_ntop(AF_INET, &cliaddr.sin_addr, cliaddrip, INET_ADDRSTRLEN);

	fprintf(stdout, "---Request comes, IP=%s\n", cliaddrip);

	/* process the request in a subprocess */
	if ((pid=fork()) == 0) {
	    pid = getpid();
	    fprintf(stdout, "---pid=%d, Process the request in a subprocess\n", pid);

	    fprintf(stdout, "---pid=%d, Close proxy socket in the subprocess\n", pid);
	    Close(proxyfd);

	    do_proxy(clifd, &cliaddr, pid);

	    fprintf(stdout, "---pid=%d, Close the client socket in the subprocess\n", pid);
	    Close(clifd);

	    fprintf(stdout, "---pid=%d, Subprocess exits\n", pid);
	    exit(0);
	}

	fprintf(stdout, "---Close the client socket in the parent process\n");
	Close(clifd);
    }

    return 0;
}
    
