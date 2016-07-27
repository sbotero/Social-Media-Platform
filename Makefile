PORT=53316
CFLAGS = -Wall -g -std=c99 -Werror

friends_server: server.o friendme.o friends.o error.o
	gcc $(CFLAGS) -o friends_server server.o friendme.o friends.o error.o

server.o: server.c friends.h error.h
	gcc $(CFLAGS) -c server.c

friendme.o: friendme.c friends.h error.h
	gcc $(CFLAGS) -c friendme.c

friends.o: friends.c friends.h error.h
	gcc $(CFLAGS) -c friends.c

error.o: error.c error.h
	gcc $(CFLAGS) -c error.c

clean: 
	rm friend_server *.o