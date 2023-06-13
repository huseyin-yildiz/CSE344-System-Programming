#ifndef CLIENT_OPERATIONS_H
#define CLIENT_OPERATIONS_H


void client_get_file(int socket_fd, const char* file_path, const char* dest_dir);
void client_send_file(int socket_fd, const char* file_path);
void client_remove_file_on_server(int socket_fd, const char* file_path);
void client_update_file_on_server(int socket_fd, const char* directory, const char* file_path);



#endif // !CLIENT_OPERATIONS_H

