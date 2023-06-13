#include "server_operations.h"


void server_send_file(int socket_fd, const char* directory) {
    char* file_path = receive_message(socket_fd);
    if (file_path == NULL) {
        perror("server_get_file receive_message");
        return;
    }

    char dest_path[MAX_FILE_PATH];
    sprintf(dest_path, "%s/%s",directory,file_path);
    
    send_file(socket_fd, dest_path);

    free(file_path);
}

void server_get_file(int socket_fd, const char* directory){
    char* file_path = receive_message(socket_fd);
    if (file_path == NULL) {
        perror("server_get_file receive_message");
        return;
    }
    
    receive_file(socket_fd, file_path, directory);

    free(file_path);    

}


void server_remove_file(int socket_fd, const char* directory){
    
    // Get relative file path
    char* file_path = receive_message(socket_fd);
    if (file_path == NULL) {
        perror("server_get_file receive_message");
        return;
    }

    // dest path
    char dest_path[MAX_FILE_PATH];
    sprintf(dest_path, "%s/%s",directory,file_path);
    
    // Remove the file path
    remove(dest_path);

    free(file_path);
}


void server_update_file(int socket_fd, const char* directory){
    
    server_get_file(socket_fd, directory);

}

