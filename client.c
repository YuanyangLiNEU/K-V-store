#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include "util.h"

int requestServerPort(int coport, char *key);
void requestServerStat(int coport, char *response);
void requestGet(int seport, char *key, char *value);
int requestSet(int seport, char *key, char *value);

int main(int argc, char *argv[]) {
   if (argc < 2) {
      printf("please input coordinator port.\n");
      return 0;
   }
   int coport = atoi(argv[1]);

   char opt[256], key[256], value[256];
   while (1) {
      bzero(opt, 256);
      bzero(key, 256);
      bzero(value, 256);

      printf("Please select the opt type [get|set|stat|quit]: ");
      fgets(opt, 255, stdin);
      opt[strcspn(opt, "\r\n")] = 0; 

      if (strcmp(opt, "quit") == 0) {
         printf("The end.\n");
         break;
      } else if (strcmp(opt, "stat") == 0) {
         char response[4096];
         bzero(response, 4096);
         requestServerStat(coport, response);
         printf("%s", response);
      } else if (strcmp(opt, "set") == 0 || strcmp(opt, "get") == 0) {
         printf("Please input the key: ");
         fgets(key, 255, stdin);
         key[strcspn(key, "\r\n")] = 0; 
         int seport = requestServerPort(coport, key);
         printf("Have got port number from coordinator and try to connect to port: %d\n", seport);

         if (strcmp(opt, "set") == 0) {
            printf("Please input the value: ");
            fgets(value, 255, stdin);
            value[strcspn(value, "\r\n")] = 0; 

            int ret = requestSet(seport, key, value);
            if (ret) {
               printf("ok\n");
            } else {
               printf("set error.\n");
            }
         } else {
            requestGet(seport, key, value);
            printf("%s\n", value);
         }
      } else {
         // continue;
      }

   }

   return 0;
}

int requestServerPort(int coport, char *key) {
   int sockfd = initClient(coport);

   // send command
   char command[] = "query";
   mywrite(sockfd, command, strlen(command));

   // get response
   char next[256];
   myread(sockfd, next, 255);

   // send key
   mywrite(sockfd, key, strlen(key));

   // get server port
   char response[256];
   bzero(response, 256);
   myread(sockfd, response, 255);

   int seport = atoi(response);
   printf("server port: %d\n", seport);

   return seport;
}

void requestServerStat(int coport, char *response) {
   int sockfd = initClient(coport);

   // send command
   char command[] = "stat";
   mywrite(sockfd, command, strlen(command));

   myread(sockfd, response, 4095);
}

void requestGet(int seport, char *key, char *value) {
   int sockfd = initClient(seport);

   char command[] = "get";
   mywrite(sockfd, command, strlen(command));

   // get response
   char next[256];
   myread(sockfd, next, 255);

   mywrite(sockfd, key, strlen(key));

   myread(sockfd, value, 127);
}

int requestSet(int seport, char *key, char *value) {
   int sockfd = initClient(seport);

   char command[] = "set";
   mywrite(sockfd, command, strlen(command));

   // get response
   char next[256];
   myread(sockfd, next, 255);

   mywrite(sockfd, key, strlen(key));

   // get response so we can transfer value
   myread(sockfd, next, 255);

   mywrite(sockfd, value, strlen(value));

   char response[256];
   bzero(response, 256);
   myread(sockfd, response, 255);

   return strcmp(response, "ok") == 0;
}