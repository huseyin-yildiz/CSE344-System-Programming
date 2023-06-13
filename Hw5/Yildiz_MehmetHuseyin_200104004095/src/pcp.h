# ifndef PCP_H
# define PCP_H

# include <stdio.h>
# include <sys/stat.h>
# include <dirent.h>
# include <string.h>
# include <pthread.h>
# include <semaphore.h>
# include "queue.h"
# include <fcntl.h>
# include <unistd.h>
# include <stdatomic.h>
# include <sys/time.h>
# include <signal.h>

# define PATH_MAX 4096
# define MAX_FILE_NAME 255
# define CP_BUFFER_SIZE 1024

typedef struct{
    int src_fd;
    int dest_fd;
    char name[MAX_FILE_NAME];
}buffer_entry;

typedef struct{
    char * dest_path;
    char * src_path;
}paths;


// Create directory recursively
void make_dir(char *dir);

// Add the file operation to the buffer 
void add_buffer(char *src_path, char *dest_path, char *file_name);

// Add all the files to the buffer recursively
void* produce(void* path);

// Read from buffer and make the copy operation
void* consume();



# endif // !PCP_H


