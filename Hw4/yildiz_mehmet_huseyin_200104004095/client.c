#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>


# define MAX_COMMAND_LENGTH 1024
# define MAX_ARGS 32
# define TEMP_DIR "/tmp/biboServer"  // Template directory
# define MAX_FILE_NAME_LEN 256
# define BUFFER_SIZE 1024


typedef struct 
{
    int in_fd;
    int out_fd;

    char server_path[MAX_FILE_NAME_LEN];
    char server_semaphore_name[MAX_FILE_NAME_LEN];
    char server_mutex_name[MAX_FILE_NAME_LEN];
    
    sem_t *sem_server;
    sem_t *write_mutex;

    char client_path[MAX_FILE_NAME_LEN];


}client;

int exit_now = 0;
int send_quit = 0;


// Takes pid and try variables and connect to the server and sets the in and out fd
int connect(client *myclient, int try){
    
    // Open server connection semaphore
    myclient->sem_server = sem_open(myclient->server_semaphore_name, O_RDWR);

    if (myclient->sem_server == SEM_FAILED) {
        perror("sem_server_open");
        exit(EXIT_FAILURE);
    }
    
    // Open server write mutex (binary semaphore)
    myclient->write_mutex = sem_open(myclient->server_mutex_name, O_RDWR);

    if (myclient->write_mutex == SEM_FAILED) {
        perror("sem_write_open");
        exit(EXIT_FAILURE);
    }


    // Open server fifo
    myclient->out_fd = open(myclient->server_path, O_WRONLY);

    
    // Wait the connection semaphore
    if(try)
    {  if(sem_trywait(myclient->sem_server) == -1)
        {
            printf("> Server Busy \n");
            exit(EXIT_SUCCESS);
        }
    } 
    else
    {
        printf("Waiting for Que..\n");
        sem_wait(myclient->sem_server);
    }
    

    printf("> Connection Established \n");
    
    // Create client fifo
    mkfifo(myclient->client_path, 0666);

    // send_request(myclient, "0");
}


// Signal handler function
void sigintHandler(int sig) {

    exit_now = 1;
    send_quit = 1;
}


void init_client(client *myclient, char *pid){

    sprintf(myclient->client_path, "%s/%d", TEMP_DIR, getpid() );
    sprintf(myclient->server_path, "%s/%s", TEMP_DIR, pid );
    

    sprintf(myclient->server_mutex_name, "%s.mtx", pid );
    sprintf(myclient->server_semaphore_name, "%s.sem", pid );
    
    struct sigaction sa;
    sa.sa_handler = sigintHandler;
    sigemptyset(&(sa.sa_mask));
    sigaddset(&(sa.sa_mask), SIGINT);
    sigaction(SIGINT, &sa, NULL);


}


// Take arguments, check them and return if boolean value for try
short arg_check(int argc, char const *argv[]){


    if (argc != 3) {
        printf("Wrong usage:\n");
        printf("Usage: biboClient <Connect/tryConnect> ServerPID\n");
        exit(EXIT_FAILURE);
    }

    // First argument is "tryConnect"
    if (strcmp(argv[1], "tryConnect") == 0) 
        return 1;  
    
    // First argument is  "Connect"
    else if(strcmp(argv[1], "Connect") == 0) 
        return 0;  
    
    printf("Wrong usage:\n");
    printf("Usage: biboClient <Connect/tryConnect> ServerPID\n");

    exit(EXIT_FAILURE);

}


void send_request(client *myclient, char *command){

    // Wait the write-mutex
    sem_wait( myclient->write_mutex );

    pid_t pid = getpid();

    int msg_len = strlen(command) + 1;

    // Write the pid
    write(myclient->out_fd, &pid, sizeof(pid_t) );

    // Write the msg len
    write(myclient->out_fd, &msg_len, sizeof(int));

    // Write the msg
    write(myclient->out_fd, command, msg_len);

    // Post the write-mutex
    sem_post(myclient->write_mutex);

}


void print_response(client *myclient, int fd){

    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    // Open the response path
    myclient->in_fd = open(myclient->client_path, O_RDONLY , 0666);

    // Read response
    while ( (bytes_read = read(myclient->in_fd, buffer, sizeof(buffer))) > 0 )
        // Print the response
        write(fd, buffer, bytes_read);   
    
    close(myclient->in_fd);

    myclient->in_fd = -1;

}



// Download the file from server
void download_file(char *filename, client *myclient){


    // Check if the file already exists
    int suffix = 0;
    char newFilename[256];
    char extension[256] = {0};
    strcpy(newFilename, filename);


    // Find extension
    char *dot = strchr(filename,'.');
    if(dot != NULL)
    {
        strcpy(extension,dot);
        *dot = '\0';
    }
    
    while (access(newFilename, F_OK) != -1) {
        suffix++;
        sprintf(newFilename, "%s%d%s", filename, suffix, extension);
    }

    // Open the file for writing
    int file = open(newFilename, O_WRONLY | O_CREAT| O_TRUNC, S_IRUSR | S_IWUSR);
    if (file == -1) {
        perror("Failed to open file");
        return;
    }

    print_response(myclient, file);

    // Close the file
    close(file);


}



// send file to server
void upload(char *filename){

    
    // char upload_path[MAX_COMMAND_LENGTH];
    
    usleep(100 * 1000);
    // Open the file for reading
    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("Upload");
        return;
    }

    // Read and write the file content
    char buffer[MAX_COMMAND_LENGTH];
    ssize_t bytesRead;
    ssize_t bytesWritten;

    char file_path[MAX_FILE_NAME_LEN];

    sprintf(file_path, "%s/%d.d",TEMP_DIR, getpid());
    mkfifo(file_path, 0666);
    int out_fd = open(file_path, O_WRONLY);

    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(out_fd, buffer, bytesRead);
        if (bytesWritten == -1) {
            perror("Failed to write to file descriptor");
            break;
        }
    }

    // Close the file
    close(file);
    
    close(out_fd);

    fflush(stdout);  
    
}




int main(int argc, char const *argv[])
{
    client myclient;
    char command[MAX_COMMAND_LENGTH];
    

    // Check the arguments
    int try = arg_check(argc, argv);

    // Initialize the client structure with arguments
    init_client(&myclient, argv[2]);

    // Test
    // char pid[30];
    // printf("Enter pid:");
    // scanf("%s",pid);
    // init_client(&myclient, pid);
    // int try = 0;
    // fgets(command, sizeof(command), stdin);

    
    // Connect to the server
    connect(&myclient, try);



    while ( ! exit_now ) {
        
    
        // Read command from the user
        printf("\n> ");
        
        fflush(stdout);

        // Take command from user
        fgets(command, sizeof(command), stdin);
        
        int i = strlen(command)-1;
        command[i] = '\0';

        // Send the request to the server
        send_request(&myclient, command);


        if(strncmp("upload",command,6) == 0 )
        {
            myclient.in_fd = open(myclient.client_path, O_RDONLY , 0666);
            upload(&command[7]);

        }
        if(strncmp("download",command,8) == 0 )
        {
            download_file(&command[9], &myclient);
            printf("Download Completed\n");
            continue;
        }
        if(strncmp("quit",command,4) == 0 )
        {
            exit_now = 1;
        }

        if(strncmp("killServer",command,10) == 0 ){
            printf("Kill request sent to server.\n");
            break;
        }

        // Take the response and print
        print_response(&myclient, STDOUT_FILENO);


    }

    if(send_quit){
        send_request(&myclient, "quit");
        print_response(&myclient, STDOUT_FILENO);
    }
    
    printf("Sending write request to Server side log file\n");
    printf("bye... \n");

    sem_post(myclient.sem_server);
    sem_close(myclient.sem_server);

    return 0;
}


