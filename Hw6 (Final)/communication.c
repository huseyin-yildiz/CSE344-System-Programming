#include "communication.h"

void send_message(int socket_fd, const char* message) {
    // Calculate the length of the message
    size_t message_length = strlen(message) + 1;

    // Send the message length to the server
    ssize_t result = send(socket_fd, &message_length, sizeof(message_length), 0);
    if (result == -1) {
        perror("send_message send message_length");
        return;
    }

    // Send the message to the server
    result = send(socket_fd, message, message_length, 0);
    if (result == -1) {
        perror("send_message send message");
        return;
    }
}

char* receive_message(int socket_fd) {
    // Receive the message length from the server
    size_t message_length;
    ssize_t result = recv(socket_fd, &message_length, sizeof(message_length), 0);
    if (result == -1) {
        perror("receive_message recv message_length");
        return NULL;
    }

    // Allocate memory for the message
    char* message = (char*)malloc(message_length);
    if (message == NULL) {
        perror("receive_message malloc");
        return NULL;
    }

    // Receive the message from the server
    result = recv(socket_fd, message, message_length, 0);
    if (result == -1) {
        perror("receive_message recv message");
        free(message);
        return NULL;
    }

    return message;
}



void receive_file(int socket_fd, const char* file_path, const char* dest_dir) {
    size_t file_size;
    ssize_t result = recv(socket_fd, &file_size, sizeof(file_size), 0);
    if (result == -1) {
        perror("receive_file recv file_size");
        return;
    }

    char dest_path[MAX_FILE_PATH];
    sprintf(dest_path, "%s/%s",dest_dir,file_path);
    char *directory = strdup(dest_path);
    char * last = strrchr(directory,'/');
    *last = '\0';
    make_dir(directory);
    free(directory);
    directory = NULL;
    
    FILE* file = fopen(dest_path, "wb");
    if (file == NULL) {
        perror("receive_file fopen");
        return;
    }

    char buffer[1024];
    size_t bytesReceived = 0;
    while (bytesReceived < file_size) {
        ssize_t bytesRead = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (bytesRead == -1) {
            perror("receive_file recv file contents");
            fclose(file);
            return;
        }

        size_t bytesWritten = fwrite(buffer, 1, bytesRead, file);
        if (bytesWritten < bytesRead) {
            perror("receive_file fwrite");
            fclose(file);
            return;
        }

        bytesReceived += bytesRead;
    }

    fclose(file);
}




void send_file(int socket_fd, const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("send_file fopen");
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    ssize_t result = send(socket_fd, &file_size, sizeof(file_size), 0);
    if (result == -1) {
        perror("send_file send file_size");
        fclose(file);
        return;
    }

    char buffer[1024];
    size_t bytesSent = 0;
    while (bytesSent < file_size) {
        size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
        if (bytesRead == 0) {
            if (feof(file)) {
                break;  // End of file
            } else {
                perror("send_file fread");
                fclose(file);
                return;
            }
        }

        ssize_t sendResult = send(socket_fd, buffer, bytesRead, 0);
        if (sendResult == -1) {
            perror("send_file send file contents");
            fclose(file);
            return;
        }

        bytesSent += sendResult;
    }

    fclose(file);
}
