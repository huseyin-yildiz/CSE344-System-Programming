#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "queue.h"
#include <unistd.h>
#include "file_list.h"
#include "operations.h"
#include "server_operations.h"

#define MAX_CLIENTS 50
#define QUEUE_BUFFER_SIZE 256 

queue buffer_queue;
char *directory;



void *client_handler(void *arg) {
    
    ssize_t result = 0;
    int connection_sock;
    Operation operation;
    
    while(1){
      if(result == 0)
      {
        connection_sock = dequeue(&buffer_queue);
        if(connection_sock == 0)
          break;
      }
      

      result = recv(connection_sock, &operation, sizeof(Operation), 0);
      if (result == -1) {
          // Handle error
          return -1;
      }
      
      // Client connection end
      if(result == 0){
        break;
      }
  
      FileInfo* file_list = NULL;
      int count = 0;
      switch (operation) {
          case OPERATION_GET_FILES_LIST:
              
              traverse_directory(directory,&file_list,&count);
              send_file_list(connection_sock, file_list, count);

              break;
          case OPERATION_GET_FILE:
              server_send_file(connection_sock, directory);
              break;
          case OPERATION_SEND_FILE:
              server_get_file(connection_sock, directory);
              break;
          case OPERATION_REMOVE_FILE:
              server_remove_file(connection_sock, directory);
              break;
          case OPERATION_UPDATE_FILE:
              server_update_file(connection_sock, directory);
              break;
          default:
              printf("Error: Unknown command get \n");
              // Handle unknown operation
              break;
      }
    }
  
  return 0;
}



// void *thread_func(void *arg) {
  

//   // Synchronize the directories on the server and client sides.
//   while (1) {

//     // Wait for a file operation from the client.
//     char operation[10];
//     int fd;
//     FILE *file;

//     if (read(info->socket, operation, sizeof(operation)) < 0) {
//       perror("read");
//       exit(1);
//     }

//     switch (operation[0]) {
//       case 'c': // Create a file.
//         fd = open(operation + 1, O_CREAT | O_WRONLY | O_TRUNC, 0666);
//         if (fd < 0) {
//           perror("open");
//           exit(1);
//         }

//         file = fdopen(fd, "w");
//         if (file == NULL) {
//           perror("fdopen");
//           exit(1);
//         }

//         fwrite(operation + 1, 1, strlen(operation + 1), file);
//         fclose(file);
//         break;

//       case 'd': // Delete a file.
//         if (remove(operation + 1) < 0) {
//           perror("remove");
//           exit(1);
//         }
//         break;

//       case 'u': // Update a file.
//         fd = open(operation + 1, O_RDWR);
//         if (fd < 0) {
//           perror("open");
//           exit(1);
//         }

//         file = fdopen(fd, "r+");
//         if (file == NULL) {
//           perror("fdopen");
//           exit(1);
//         }

//         fwrite(operation + 1, 1, strlen(operation + 1), file);
//         fclose(file);
//         break;
//     }
//   }
// }

int main(int argc, char *argv[]) {


  int port, thread_pool_size;
  struct sockaddr_in server_addr;
  int server_socket, client_socket;
  pthread_t *threads;
  int i;


  
  // Check the number of arguments.
  // if (argc != 4) {
  //   fprintf(stderr, "Usage: %s directory thread_pool_size port\n", argv[0]);
  //   exit(1);
  // }

  // Get the directory, thread pool size, and port number.
  directory = "test"; // argv[1];
  thread_pool_size = 5; // atoi(argv[2]);
  port = 1454; // atoi(argv[3]);

  
  init_queue(&buffer_queue, QUEUE_BUFFER_SIZE);
  
  // Create the server socket.
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("socket");
    exit(1);
  }

  // Bind the server socket to the port number.
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    exit(1);
  }

  // Listen for connections on the server socket.
  listen(server_socket, 5);                                         // !!!! burda 5 yerine threadpool size olmalıydı bence

  // Create the threads.
  threads = malloc(sizeof(pthread_t) * thread_pool_size);
  
  for (i = 0; i < thread_pool_size; i++) {
    pthread_create(&threads[i], NULL, client_handler, NULL);
  }

  // Accept connections from clients.
  while (1) {
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket < 0) {
      perror("accept");
      exit(1);
    }

    enqueue(&buffer_queue, client_socket);
    
  }

  // Cleanup.
  for (i = 0; i < thread_pool_size; i++) {
    pthread_join(threads[i], NULL);
  }
  free(threads);
  close(server_socket);

  return 0;
}
