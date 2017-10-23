#include "coordinator.h"

int main(int argc, char *argv[]) {
	int port = DEFAULT_PORT;
	if (argc > 1) {
		port = atoi(argv[1]);
	}

	initNodes();
	createCoodinator(port);
}

void initNodes() {
	for (int i = 0; i < MAX_SERVER; ++i) {
		nodes[i] = -1;
	}
}

void createCoodinator(int port) {
    printf("start coordinator on port: %d\n", port);
    int sockfd = initServer(port);

    while (1) {
       int newsockfd = accept(sockfd, (struct sockaddr *) NULL, NULL);
      
       if (newsockfd < 0) {
          perror("ERROR on accept");
          exit(1);
       }

       processMsg(newsockfd);
       //close(newsockfd);
    }
}

void processMsg(int sock) {
    printf("start process msg\n");
    char command[256];
    bzero(command, 256);
    myread(sock, command, 255);
   
    printf("Command: %s\n", command);

    if (strcmp(command, "stat") == 0) {
    	char response[4096];
    	bzero(response, 4096);
    	sprintf(response, "total: %d\n", MAX_SERVER);
    	for (int i = 0; i < MAX_SERVER; ++i) {
    		if (nodes[i] > 0) {
    			char line[128];
    			bzero(line, 128);
    			sprintf(line, "pos: %d, port: %d\n", i, nodes[i]);
    			strcat(response, line);
    		}
    	}

    	mywrite(sock, response, strlen(response));

    	return ;
    }

    char next[] = "next";
    mywrite(sock, next, strlen(next));

   	// read content
   	char content[256];
   	bzero(content, 256);
   	myread(sock, content, 255);

   	printf("Content: %s\n", content);

   	if (strcmp(command, "register") == 0) {
   		int regport = atoi(content);
   		processRegMsg(sock, regport);
   	} else if (strcmp(command, "unregister") == 0) {
   		int unregpos = atoi(content);
   		processUnregMsg(sock, unregpos);
   	} else if (strcmp(command, "query") == 0) {
   		processQueryMsg(sock, content);
   	}
}

void processRegMsg(int sock, int regport) {
   	int cur = -1;
   	int nextport = -1;
   	for (int i = 0; i < MAX_SERVER; ++i) {
   		if (nodes[i] == -1) {
   			nodes[i] = regport;
   			cur = i;
   			break;
   		}
   	}

   	if (cur != -1) {
   		int next = (cur + 1) % MAX_SERVER;
   		while (next != cur) {
   			if (nodes[next] > 0) {
   				nextport = nodes[next];
   				break;
   			}
   			next = (next + 1) % MAX_SERVER;
   		}
   	}

   	//send cur server slot and port of next server to client, error return -1,-1
	char response[32];
    sprintf(response, "%d,%d", cur, nextport);
   	mywrite(sock, response, strlen(response));
}

void processUnregMsg(int sock, int unregpos) {
   	int ret = -1;
   	if (unregpos != -1) {
   		nodes[unregpos] = -1;
	   	int next = (unregpos + 1) % MAX_SERVER;
	   	while (next != unregpos) {
	   		if (nodes[next] > 0) {
	   			ret = nodes[next];
	   			break;
	   		}
	   		next = (next + 1) % MAX_SERVER;
	   	}
   	}

   	//send port of next server, no server found return -1
	char response[32];
    sprintf(response, "%d", ret);
   	mywrite(sock, response, strlen(response));
}

void processQueryMsg(int sock, char* key) {
	int hash = computeHash(key) % MAX_SERVER;
	int ret = -1;
	if (nodes[hash] > 0) {
		ret = nodes[hash];
	} else {
		int pos = (hash + 1) % MAX_SERVER;
		while (pos != hash) {
			if (nodes[pos] > 0) {
				ret = nodes[pos];
				break;
			}
			pos = (pos + 1) % MAX_SERVER;
		}
	}

	//send server port to client, no server found return -1
	char response[32];
    sprintf(response, "%d", ret);
   	mywrite(sock, response, strlen(response));
}