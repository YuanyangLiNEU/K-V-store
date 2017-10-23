all: client server coordinator
	
client.o: client.c
	gcc -c -Wall -Werror -fpic -o client.o client.c
client: client.o util.o
	gcc -g -o client client.o util.o

coordinator.o: coordinator.c
	gcc -c -Wall -Werror -fpic -o coordinator.o coordinator.c
coordinator: coordinator.o util.o
	gcc -g -o coordinator coordinator.o util.o

server.o: server.c
	gcc -c -Wall -Werror -fpic -o server.o server.c -lpthread
server: server.o hashtable.o util.o
	gcc -g -o server server.o hashtable.o util.o -lpthread

hashtable.o: hashtable.c
	gcc -c -Wall -Werror -fpic -o hashtable.o hashtable.c

util.o: util.c
	gcc -c -Wall -Werror -fpic -o util.o util.c

clean: 
	rm -rf client client.o coordinator coordinator.o server server.o hashtable.o util.o
dist:
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
