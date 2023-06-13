pCp:
	${CC} main.c queue.c -lpthread -o pCp

clean:
	${RM} pCp 

val:pCp
	valgrind -s --leak-check=full ./pCp
