#include "util.h"

int computeHash(char *key) {
   int h = 0;
   char *p = key;
   while (*p != '\0') {
      h = 31 * h + *p;
      ++p;
   }

   return abs(h);
}

int initServer(int port) {
   int sockfd, portno;
   struct sockaddr_in serv_addr;
   
   // First call to socket() function
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   // Initialize socket structure
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = port;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   // Now bind the host address using bind() call
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
   
   listen(sockfd, 5);
   
   return sockfd;
}

int initClient(int port) {
   int sockfd;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   server = gethostbyname("localhost");
   
   if (server == NULL) {
      perror("ERROR, no such host");
      exit(1);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(port);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }

   return sockfd;
}

ssize_t myread(int fd, void *buf, size_t count) {
	ssize_t n = read(fd, buf, count);
	if (n < 0) {
      	perror("ERROR reading from socket");
      	exit(1);
   	}

   	return n;
}

ssize_t mywrite(int fd, void *buf, size_t count) {
	ssize_t n = write(fd, buf, count);
	if (n < 0) {
		perror("ERROR writing to socket");
      	exit(1); 
	}

	return n;
}