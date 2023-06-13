#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "operations.h"
#include "file_list.h"
#include "client_operations.h"

int main(int argc, char *argv[]) {
  int port;
  char *directory;
  struct sockaddr_in server_addr;
  int client_socket;

  // // Check the number of arguments.
  // if (argc != 3) {
  //   fprintf(stderr, "Usage: %s directory port\n", argv[0]);
  //   exit(1);
  // }

  // Get the directory and port number.
  directory = "a"; // argv[1];
  port = 1454; // atoi(argv[2]);

  // Create the client socket.
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    perror("socket");
    exit(1);
  }

  // Connect to the server.
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("connect");
    exit(1);
  }

  
  Operation operation = OPERATION_GET_FILES_LIST;
  int count;
  ssize_t result = send(client_socket, &operation, sizeof(Operation), 0);
  FileInfo *file_list = receive_file_list(client_socket, &count);
  
  // Print the file paths and hashes
    for (int i = 0; i < count; i++) {
        printf("File Path: %s\n", file_list[i].path);
        // client_get_file(client_socket, file_list[i].path, directory);

        printf("\n\n");
    }

    client_send_file(client_socket, "final_project.pdf");
    sleep(5);
    client_remove_file_on_server(client_socket, "final_project.pdf");
    client_update_file_on_server(client_socket, "a", "asdasd");

  // // Send the directory name to the server.
  // if (write(client_socket, directory, strlen(directory)) < 0) {
  //   perror("write");
  //   exit(1);
  // }

  // // Wait for a response from the server.
  // char response[10];
  // int bytes_read;
  // bytes_read = read(client_socket, response, sizeof(response));
  // if (bytes_read < 0) {
  //   perror("read");
  //   exit(1);
  // }

  // // Check the response from the server.
  // if (strcmp(response, "OK") != 0) {
  //   fprintf(stderr, "Error: Server responded with %s\n", response);
  //   exit(1);
  // }

  // Perform file operations on the server.
  // ...

  // Close the client socket.
  close(client_socket);

  return 0;
}
