#ifndef FILE_TRAVERSAL_H
#define FILE_TRAVERSAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX_PATH_LENGTH PATH_MAX
#define SHA256_DIGEST_LENGTH 32

typedef struct {
    char path[MAX_PATH_LENGTH];
    unsigned char hash[SHA256_DIGEST_LENGTH];
} FileInfo;

void traverse_directory(const char* directory_path, FileInfo** file_list, int* count);
void calculate_file_hash(const char* file_path, unsigned char* hash);
void send_file_list(int socket_fd, FileInfo* file_list, int count);
FileInfo* receive_file_list(int socket_fd, int* count);
void make_dir(char *dir);

#endif /* FILE_TRAVERSAL_H */
