main: BibakBOXServer BibakBOXClient

BibakBOXServer: clean
	gcc server.c file_list.c communication.c queue.c server_operations.c -pthread -lcrypto -o BibakBOXServer

BibakBOXClient: clean
	gcc client.c file_list.c communication.c  client_operations.c -pthread -lcrypto -o BibakBOXClient 

file_list: install_openssl clean
	gcc -o file_list file_list.c -lcrypto

install_openssl:
	sudo apt-get install libssl-dev

clean:
	rm -f BibakBOXClient BibakBOXServer file_list
