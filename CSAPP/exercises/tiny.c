#include "csapp.h"
#define STATIC 0
#define DYNAMIC 1
#define MAXLEN 100

void doit(int fd);
void skipRstHdrs(rio_t *rio);   // skip request headers

/*
    parse url, get the filename.
    if the url is static, then cgiargs is null, return STATIC.
    otherwise, get the cgiargs and return DYNAMIC;
*/
int parseURL(const char *url, char *filename, char *cgiargs);   
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void serverStatic(int fd, char *filename, int size);
void getFilename(const char *filename, char *type);

int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in *clientaddr;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(struct sockaddr_in);
        connfd = Accept(listenfd, (SA *)clientaddr, &clientlen);
        doit(connfd);
        Close(connfd);
    }
    return 0;
}

void doit(int fd) {
    char method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, url, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implement", "This function has not implemented");
        return;
    }
    skipRstHdrs(&rio);
    
    char filename[MAXLEN], cgiargs[MAXLEN];
    struct stat sbuf;
    
    int requestType = parseURL(url, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not Found", "Can not found this file");
        return;
    }
    if (requestType == STATIC) {
        if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR && sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Can not open this file");
            return;
        }
        serveStatic(fd, filename, sbuf.st_size);
    }
    else {
        if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR && sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Can not open this file");
            return;
        }
        serveDynamic(fd, filename, cgiargs);
    }
        
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];
    
    // build the HTTP response body
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web Server</em>\r\n", body);
    
    // print the HTTP response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void skipRstHdrs(rio_t *rio) {
    char buf[MAXLINE];
    Rio_readlineb(rio, buf, MAXLINE);
    while (strcmp("\r\n", buf)) {
        Rio_readlineb(rio, buf, MAXLINE);
        printf("%s", buf);
    }
}

int parseURL(const char *url, char *filename, char *cgiargs) {
    
    strcpy(filename, ".");
    if (!strstr(url, "/cgi-bin")) {   //static
        strcat(filename, url);
        cgiargs = NULL;
        return STATIC;
    }
    else {
        char *ptr = index(url, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';    // so that we can get filename
        }
        strcat(filename, url);
        return DYNAMIC;
    }
}

void getFiletype(const char *filename, char *filetype) {
    if (strstr(filename, ".html")) 
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jgp"))
        strcpy(filetype, "image/jpg");
    else
        strcpy(filetype, "text/plain");
}

void serveStatic(int fd, char *filename, int size) {
    int srcfd;
    char *srcp, filetype[MAXLEN], buf[MAXBUF];

    getFiletype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, size);
    Rio_writen(fd, buf, strlen(buf));
    
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, size, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, size);
    Munmap(srcp, size);
}

void serveDynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXLINE];
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%sServer: Tiny Server\r\n", buf);
    Rio_writen(fd, buf, strlen(buf));
    
    char *args[] = {NULL};
    if (Fork() == 0) {
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);
        Execve(filename, args, environ);
    }
    wait(NULL);
}




        
    
    
    
    
    
        
        
    
