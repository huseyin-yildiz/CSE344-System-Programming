main:server client

server:clean
	$(CC) server.c queue.c -w -lpthread -o biboServer



client: clean
	$(CC) client.c -w -lpthread -o biboClient


clean:
	${RM} biboServer biboClient


