#ifndef __util_h__
#define __util_h__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#ifndef MAX_SERVER
#define MAX_SERVER 2
#endif

// hash computation
int computeHash(char *key);

// network
int initServer(int port);
int initClient(int port);
ssize_t myread(int fd, void *buf, size_t count);
ssize_t mywrite(int fd, void *buf, size_t count);

#endif