#ifndef SERVER_OPERATIONS_H
#define SERVER_OPERATIONS_H

#include "communication.h"

void server_send_file(int socket_fd, const char* directory);
void server_get_file(int socket_fd, const char* directory);
void server_remove_file(int socket_fd, const char* directory);
void server_update_file(int socket_fd, const char* directory);


#endif // !SERVER_OPERATIONS_H


