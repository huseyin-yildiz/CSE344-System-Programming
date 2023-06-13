#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "file_list.h"

#define MAX_FILE_PATH 1024

void send_message(int socket_fd, const char* message);
char* receive_message(int socket_fd);
void receive_file(int socket_fd, const char* file_path, const char* dest_dir);
void send_file(int socket_fd, const char* file_path);

#endif /* COMMUNICATION_H */
