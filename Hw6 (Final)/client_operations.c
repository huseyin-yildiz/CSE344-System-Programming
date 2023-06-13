#include "communication.h"
#include "operations.h"
#include "client_operations.h"


void client_get_file(int socket_fd, const char* file_path, const char* dest_dir) {
    
    Operation operation = OPERATION_GET_FILE;
    send(socket_fd, &operation, sizeof(Operation), 0);

    send_message(socket_fd, file_path);
    receive_file(socket_fd, file_path, dest_dir);
}


void client_send_file(int socket_fd, const char* file_path){
    Operation operation = OPERATION_SEND_FILE;
    send(socket_fd, &operation, sizeof(Operation), 0);

    // send file_path
    send_message(socket_fd, file_path);
    
    // send the file
    send_file(socket_fd, file_path);
}


void client_remove_file_on_server(int socket_fd, const char* file_path){
    Operation operation = OPERATION_REMOVE_FILE;
    send(socket_fd, &operation, sizeof(Operation), 0);

    // send file_path to remove
    send_message(socket_fd, file_path);
}


void client_update_file_on_server(int socket_fd, const char* directory, const char* file_path){
    Operation operation = OPERATION_UPDATE_FILE;
    send(socket_fd, &operation, sizeof(Operation), 0);

    // send file_path
    send_message(socket_fd, file_path);
    
    // source path to send
    char source_path[MAX_FILE_PATH];
    sprintf(source_path, "%s/%s",directory,file_path);
    
    // send file
    send_file(socket_fd, source_path);
}
