#ifndef __server_h__
#define __server_h__

#include <pthread.h>
#include "util.h"
#include "hashtable.h"

int registerServer();
int unregisterServer();
void syncFromNext(int nextport);
void syncToNext(int nextport);

void startServerBg();
void* startServer(void* para);

void startCLI();

void processMsg(int sockfd);
void processSyncFromMsg(int sockfd);
void processSyncToMsg(int sockfd);
void processGetMsg(int sockfd);
void processSetMsg(int sockfd);

void get(char *key, char *value);
void set(char *key, char *value);

static Row **hashtable;
static int mypos = -1;
static int myport = -1;
static int coport = -1;

#endif