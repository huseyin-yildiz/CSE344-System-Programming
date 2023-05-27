#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>


# define MAX_COMMAND_LENGTH 1024
# define MAX_ARGS 32
# define TEMP_DIR "/tmp/biboServer"  // Template directory
# define MAX_FILE_NAME_LEN 40


typedef struct 
{
    int in_fd;
    int out_fd;

}client;


int main_server_pid = -1;
char response_path[MAX_FILE_NAME_LEN];
int exit_now = 0;

// Signal handler function
void sigintHandler(int sig) {
    printf("Received SIGINT signal. Sending kill request...\n");
    
    if(main_server_pid != -1)
        kill(main_server_pid, sig);

    exit_now = 1;
}




// Takes pid and try variables and connect to the server and sets the in and out fd
int connect(client *myclient, int try, char* pid){
    
    char server_path[MAX_FILE_NAME_LEN];
    int main_server, child_server;
    char msg[MAX_COMMAND_LENGTH];
    
    char request_path[MAX_FILE_NAME_LEN];

    // Response Fifo path
    sprintf(response_path,"%s/%d",TEMP_DIR,getpid() );
    
    // Open incoming fifo to take responses
    mkfifo(response_path, 0666);


    
    // server path 
    sprintf(server_path,"%s/%s", TEMP_DIR, pid);
    
    // open server fifo for connection request
    main_server = open(server_path, O_WRONLY , 0666);
    
    // create connection request message
    if(try)
        sprintf(msg, "t%d", getpid());
    else
        sprintf(msg, "%d", getpid());

    // send connection request to server
    write(main_server, msg, strlen(msg) + 1);


    if(try)
        myclient->in_fd = open(response_path, O_RDONLY | O_NONBLOCK, 0666);
    else
        myclient->in_fd = open(response_path, O_RDONLY , 0666);


    usleep(1000 * 10);
    // Take response
    int a = read(myclient->in_fd, msg, MAX_COMMAND_LENGTH);

    if( strncmp("wait",msg,4) == 0 )
    {   
        // Server is busy
        if(try)
            return -1;          
        
        printf("Waiting for Que..");
        close(myclient->in_fd);
        fflush(stdout);
        myclient->in_fd = open(response_path, O_RDONLY , 0666);

        // Get Child Server pid
        read(myclient->in_fd, msg, MAX_COMMAND_LENGTH);
    }
    
    // failure try
    if(a == 0 && try)
    {
        printf("Server Busy.\n");
        exit(EXIT_SUCCESS);
    }
    

        

    printf(" Server ready\n:%s",msg);

    // Request Path to send messages (Child server )
    sprintf(request_path ,"%s/%s",TEMP_DIR, msg );

    myclient->out_fd = open(request_path, O_WRONLY, 0666);

    return 0;
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


// Download the file from server
void download_file(char *filename, int in_fd){

    

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

    // Read from the file descriptor and write to the file
    char buffer[MAX_COMMAND_LENGTH];
    ssize_t bytesRead;
    ssize_t bytesWritten;

    
    while ((bytesRead = read(in_fd, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(file, buffer, bytesRead);
        if(bytesRead != sizeof(buffer))
            break;
    }

    // Close the file
    close(file);


}



// Upload the file to server
void upload_file(char *command, int out_fd){
    
    usleep(100 * 1000);
    // char upload_path[MAX_COMMAND_LENGTH];
    
    char *filename = &command[7];
    

    // sprintf(upload_path,"%s/%s",TEMP_DIR,command );
    // mkfifo(upload_path, 0666);


    // int upload_out = open(upload_path, O_WRONLY , 0666);

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

    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(out_fd, buffer, bytesRead);
        if (bytesWritten == -1) {
            perror("Failed to write to file descriptor");
            break;
        }
    }

    // Close the file
    close(file);
    
//     close(upload_out);


}






int main(int argc, char const *argv[]) {
    
    char* args[MAX_ARGS];
    char command[MAX_COMMAND_LENGTH];
    char response[MAX_COMMAND_LENGTH];
    char end_response[MAX_COMMAND_LENGTH] = {0};
    int try;

    client myclient;

    // Argument Check
    try = arg_check(argc, argv);
    
    connect(&myclient, try, argv[2]);

    // printf("Enter Server pid:");
    // char pid[MAX_COMMAND_LENGTH];
    // fgets(pid, sizeof(pid), stdin);
    // pid[strlen(pid)-1] = '\0'; 

    // connect(&myclient, 0, pid);

    // if (connect(&myclient, try, argv[2]) == -1)
    // {
    //     printf("Server busy. Quiting.. ");
    //     return EXIT_SUCCESS;
    // }

    // Signal Handler setup
    struct sigaction sa;
    sa.sa_handler = sigintHandler;
    sigemptyset(&(sa.sa_mask));
    sigaddset(&(sa.sa_mask), SIGINT);
    sigaction(SIGINT, &sa, NULL);



    while ( !exit_now ) {
        
        


        // Read command from the user
        printf("\n> ");
        
        fflush(stdout);


        // Take command from user
        fgets(command, sizeof(command), stdin);
        
        int i = strlen(command)-1;
        command[i] = '\0';

        // Send the command to the server
        write(myclient.out_fd, command, strlen(command)+1);
        // fsync(myclient.out_fd);

        if(strncmp("upload",command,6) == 0 )
            upload_file(command, myclient.out_fd);

        if(strncmp("download",command,8) == 0 )
        {
            download_file(&command[9], myclient.in_fd);
            printf("Download Completed\n");
            continue;
        }
        if(strncmp("quit",command,4) == 0 )
        {
            printf("Sending write request to Server side log file\n");
            printf("bye...");
            return EXIT_SUCCESS;
        }

        if(strncmp("killServer",command,10) == 0 )
            printf("Kill request sent to server.\n");



        

        myclient.in_fd = open(response_path, O_RDONLY , 0666);
        int bytes_read;
        // Read data from the file descriptor until end of file
        // usleep(100 * 1000);
        while ((bytes_read = read(myclient.in_fd, response, sizeof(response))) > 0) {
            // Write the data to stdout
            write(STDOUT_FILENO, response, bytes_read);
            // if(bytes_read == sizeof(response) && ! strcmp(end_response, response))
            //     break;

            // Check if the newline character is encountered
            
        }
    }



    

    printf("Exiting...\n");

    return 0;
}
