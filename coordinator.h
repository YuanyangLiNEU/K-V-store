#ifndef __coordinator_h__
#define __coordinator_h__

#include "util.h"

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 12131
#endif

// init
void initNodes();
void createCoodinator(int port);

// process message
void processMsg(int sock);
void processRegMsg(int sock, int regport);
void processUnregMsg(int sock, int unregpos);
void processQueryMsg(int sock, char *key);

static int nodes[MAX_SERVER];

#endif