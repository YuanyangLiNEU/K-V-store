#include "server.h"

int main(int argc, char *argv[]) {
   if (argc < 3) {
      printf("please input server port and coordinator port\n");
      return 0;
   }

   myport = atoi(argv[1]);
   coport = atoi(argv[2]);

   // init hashtable
   hashtable = initDB();

   // register this server to coordinator and get next server port
   int nextport = registerServer();

   // sync data that should be stored in this server from next server
   syncFromNext(nextport);

   // start server in background to process request
   startServerBg();

   // start a command line to execute user commands
   startCLI();
   
   return 0;
}

int registerServer() {
   int sockfd = initClient(coport);

   char command[] = "register";
   int n = write(sockfd, command, strlen(command));
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);      
   }

   char next[256];
   myread(sockfd, next, 255);

   char content[256];
   sprintf(content, "%d", myport);
   mywrite(sockfd, content, strlen(content));

   char response[256];
   bzero(response, 256);
   myread(sockfd, response, 255);

   int nextport = -1;
   sscanf(response, "%d,%d", &mypos, &nextport);
   if (mypos == -1) {
      printf("register failed.\n");
      exit(1);
   }
   printf("mypos: %d, nextport: %d\n", mypos, nextport);

   return nextport;
}

int unregisterServer() {
   int sockfd = initClient(coport);

   char command[] = "unregister";
   mywrite(sockfd, command, strlen(command));

   char next[256];
   myread(sockfd, next, 255);

   char content[256];
   sprintf(content, "%d", mypos);
   mywrite(sockfd, content, strlen(content));

   char response[256];
   bzero(response, 256);
   myread(sockfd, response, 255);

   int nextport = atoi(response);
   printf("nextport: %d\n", nextport);

   return nextport;
}

void syncFromNext(int nextport) {
   if (nextport == -1) {
      printf("First server.\n");
      return ;
   }

   int sockfd = initClient(nextport);
   char command[] = "syncfrom";
   mywrite(sockfd, command, strlen(command));

   char next[256];
   myread(sockfd, next, 255);

   char position[256];
   sprintf(position, "%d", mypos);
   mywrite(sockfd, position, strlen(position));

   bzero(next, 256);
   myread(sockfd, next, 255);

   char key[128];
   char value[128];
   while (strcmp(next, "end") != 0) {
      bzero(key, 128);
      bzero(value, 128);

      mywrite(sockfd, "key", 3);
      myread(sockfd, key, 127);
      mywrite(sockfd, "value", 5);
      myread(sockfd, value, 127);
      mywrite(sockfd, "next?", 5);

      bzero(next, 256);
      myread(sockfd, next, 255);

      setRow(hashtable, key, value);
   }
}

void syncToNext(int nextport) {
   if (nextport == -1) {
      printf("invalid next port.\n");
      return ;
   }

   int sockfd = initClient(nextport);
   char command[] = "syncto";
   mywrite(sockfd, command, strlen(command));

   char next[256];
   myread(sockfd, next, 255);

   for (int i = 0; i < TABLE_SIZE; ++i) {
      Row *p = hashtable[i];
      while (p != NULL) {
         mywrite(sockfd, "next", 4);
         myread(sockfd, next, 255);
         mywrite(sockfd, p->key, strlen(p->key));
         myread(sockfd, next, 255);
         mywrite(sockfd, p->value, strlen(p->value));
         myread(sockfd, next, 255);

         p = p->next;
      }
   }
   mywrite(sockfd, "end", 3);

   closeDB(hashtable);
}

void startCLI(int myport) {
   char opt[256];
   while (1) {
      bzero(opt, 256);

      printf("Please select the opt type [all|quit]: ");
      fgets(opt, 255, stdin);
      opt[strcspn(opt, "\r\n")] = 0; 

      if (strcmp(opt, "quit") == 0) {
         int nextport = unregisterServer();
         if (nextport == -1) {
            printf("Warnning: this is the last server. We will lost all our data.\n");
         } else {
            syncToNext(nextport);
         }
         printf("The end.\n");
         break;
      } else if (strcmp(opt, "all") == 0) {
         for (int i = 0; i < TABLE_SIZE; ++i) {
            Row *p = hashtable[i];
            while (p != NULL) {
               printf("key: %s, value: %s\n", p->key, p->value);
               p = p->next;
            }
         }
      } else {
         // continue;
      }
   }
}

void startServerBg() {
   pthread_t inc_x_thread;
   if (pthread_create(&inc_x_thread, NULL, startServer, NULL)) {
      perror("Error creating server\n");
      exit(1);
   }
}

void* startServer(void *para) {
   printf("start coordinator on port: %d\n", myport);
   int sockfd = initServer(myport);

   while (1) {
      int newsockfd = accept(sockfd, (struct sockaddr *) NULL, NULL);
      
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }

      processMsg(newsockfd);
   }

   return NULL;
}

void processMsg(int sockfd) {
   char command[256];
   bzero(command, 255);
   myread(sockfd, command, 255);

   char next[] = "next";
   mywrite(sockfd, next, strlen(next));

   //printf("command: %s\n", command);
   if (strcmp(command, "syncfrom") == 0) {
      processSyncFromMsg(sockfd);
   } else if (strcmp(command, "syncto") == 0) {
      processSyncToMsg(sockfd);
   } else if (strcmp(command, "get") == 0) {
      processGetMsg(sockfd);
   } else if (strcmp(command, "set") == 0) {
      processSetMsg(sockfd);
   }
}

void processSyncFromMsg(int sockfd) {
   char posstr[128];
   bzero(posstr, 128);
   myread(sockfd, posstr, 127);

   int pos = atoi(posstr);
   if (pos < 0 || pos >= MAX_SERVER) {
      mywrite(sockfd, "end", 3);
      return ;
   }

   char next[256];
   Row **newHashtable = initDB();
   for (int i = 0; i < TABLE_SIZE; ++i) {
      Row *p = hashtable[i];
      while (p != NULL) {
         int hash = computeHash(p->key) % MAX_SERVER;
         int curDis = (mypos + MAX_SERVER - hash) % MAX_SERVER;
         int lastDis = (pos + MAX_SERVER - hash) % MAX_SERVER;

         if (lastDis < curDis) {
            mywrite(sockfd, "next", 4);
            myread(sockfd, next, 255);
            mywrite(sockfd, p->key, strlen(p->key));
            myread(sockfd, next, 255);
            mywrite(sockfd, p->value, strlen(p->value));
            myread(sockfd, next, 255);
         } else {
            setRow(newHashtable, p->key, p->value);
         }

         p = p->next;
      }
   }
   mywrite(sockfd, "end", 3);

   closeDB(hashtable);
   hashtable = newHashtable;
}

void processSyncToMsg(int sockfd) {
   char next[256];
   bzero(next, 256);
   myread(sockfd, next, 255);

   char key[128];
   char value[128];
   while (strcmp(next, "end") != 0) {
      bzero(key, 128);
      bzero(value, 128);

      mywrite(sockfd, "key", 3);
      myread(sockfd, key, 127);
      mywrite(sockfd, "value", 5);
      myread(sockfd, value, 127);
      mywrite(sockfd, "next?", 5);

      bzero(next, 256);
      myread(sockfd, next, 255);

      setRow(hashtable, key, value);
   }
}

void processGetMsg(int sockfd) {
   char key[128];
   bzero(key, 128);
   myread(sockfd, key, 127);

   char value[128];
   get(key, value);
   mywrite(sockfd, value, strlen(value));
}

void processSetMsg(int sockfd) {
   char key[128];
   char value[128];
   bzero(key, 128);
   bzero(value, 128);

   myread(sockfd, key, 127);
   mywrite(sockfd, "next", 4);
   myread(sockfd, value, 127);

   //printf("key: %s, value: %s\n", key, value);
   set(key, value);

   char response[] = "ok";
   mywrite(sockfd, response, strlen(response));
}

void get(char *key, char *value) {
   Row *row = getRow(hashtable, key);
   if (row == NULL) {
      strcpy(value, "key not exist.");
   } else {
      strcpy(value, row->value);
   }
}

void set(char *key, char *value) {
   setRow(hashtable, key, value);
}