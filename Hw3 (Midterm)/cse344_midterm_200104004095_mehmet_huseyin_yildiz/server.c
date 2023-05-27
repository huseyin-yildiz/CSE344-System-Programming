# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>

# include "queue.h"     // Queue implementation
# include "file_handler.h"
# include "log.h"

# include <sys/stat.h>
# include <sys/types.h>
# include <signal.h>
# include <sys/wait.h>


# define MAX_FILE_NAME_LEN 40
# define MAX_CHLD_SIZE 100          // Maximum possible child size
# define REQUEST_LEN 20

typedef struct
{
    int max_client;                 // Maximum simultaneous client capacity
    const char *directory;                // The directory server run on
    int connection_fd;              // File Descriptor (Fifo) for incominq requests
    pid_t children[MAX_CHLD_SIZE];  // Children processes pid list
    queue *request_queue;           // Request queue
    int available_child_size;       // Size of available children
    char connection_path[MAX_FILE_NAME_LEN];

}server;

int exit_now  = 0;

queue req_queue = { .front = 0, .rear = 0, .size = 0 };
server myserver = {.children = {0}, .request_queue = &req_queue};

int is_parent = 1;




// Signal handler function
void sigintHandler(int sig) {
    if(!is_parent)
        printf(">> Received SIGINT signal. Quiting...\n");

    else{
        printf(">> kill signal.. terminating..\n");
        for (int i = 0; i < myserver.max_client; i++)
        {
            int pid = myserver.children[i];
            if(pid != NULL){
                kill(pid, sig);
                waitpid(pid, NULL, 0);
            }
        }
        printf(">> bye\n");
    }
    
    exit(EXIT_SUCCESS);

}



// Take arguments and check them
short arg_check(int argc, char const *argv[]){

    if(argc != 3)
        return 0;


    if( ! isdigit(argv[2]) )
        return 0;

    return 1;

}

// Handle termination signal of child
void handle_SIGCHLD(int sig, siginfo_t *info, void *context)
{
    

    int signalPid = info->si_pid;
    
    printf(">> SIGCHILD taken from %d\n",signalPid);

    for (int i = 0; i < myserver.max_client; i++)
    {
        if(myserver.children[i] == signalPid)
        {
            myserver.children[i] = 0;
            myserver.available_child_size++;

            printf(">> Child:%d is finished.\n",i);
            process_queue(&myserver);

            break;

        }

    }

}


// If a child terminated then make it available
void check_children(server* myserver){

    for (int i = 0; i < myserver->max_client; i++)
    {
        pid_t pid = myserver->children[i];

        if(pid != 0)
        {
            // Check the child without block
            int pid_result = waitpid(pid, NULL, WNOHANG);

            // If the child is terminated then make it available
            if(pid_result == pid){
                myserver->children[i] = 0;
                myserver->available_child_size++;
            }
        }

    }
}


// Takes the arguments and initialize the server struct and all required initial operations (fifo, child processes etc.)
void init_server(server* myserver, int max_cli, char *directory){



    myserver->max_client = max_cli;
    myserver->directory  = directory;

    // Create a template directory for the this app
    mkdir(TEMP_DIR, 0777);

    sprintf(myserver->connection_path, "%s/%d", TEMP_DIR, (int)getpid());

    // Create fifo for incoming requests
    mkfifo(myserver->connection_path, 0666);

    printf(">> Server Started PID %d... \n",getpid());
    printf(">> waiting for clients...\n");

    myserver->connection_fd = open(myserver->connection_path, O_RDONLY , 0666);

    // Open a dummyfd to be blocked in every read
    int dummyFd = open(myserver->connection_path, O_WRONLY);
    if (dummyFd == -1)
        perror("Dummy fd error");


    myserver->available_child_size = myserver->max_client;



    // SIGCHLD Signal Handler setup
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_handler = &handle_SIGCHLD;
    sigaction(SIGCHLD, &sa, NULL);


     // SIGINT Signal Handler setup
    struct sigaction sa1;
    sa1.sa_handler = sigintHandler;
    sigemptyset(&(sa1.sa_mask));
    sigaddset(&(sa1.sa_mask), SIGINT);
    sigaction(SIGINT, &sa1, NULL);






}


// Sleep until a connection request and return the request
// Get the request from server's fifo fd
char* get_request(server *myserver){

    char *request = malloc(REQUEST_LEN);
    read(myserver->connection_fd, (void*)request, REQUEST_LEN);

    return request;
}


void send_response(char *request, char *msg){

    char server_path[MAX_FILE_NAME_LEN];
    sprintf(server_path,"%s/%s", TEMP_DIR, request);
    int fd = open(server_path, O_WRONLY, 0666);
    write(fd,msg,strlen(msg)+1);
    close(fd);
}


// Handle the request
// If the request is 'tryConnect' and any children not available then send response to client
// otherwise add request to queue
void handle_request(server *myserver, char* request){

    // Check children to see available ones
    check_children(myserver);


    if( myserver->available_child_size < 1 )
    {
        send_response(request, "wait");
       // printf("%s Waits",request);
    }
    // If the request is 'tryConnect'
    if( request[0] != 't' | myserver->available_child_size > 0 )
    {
            enqueue(myserver->request_queue, request);
            printf(">> Client%s added to queue \n",request);
    }

}

// Dedicated function for client that takes requests and response them (For Child Processes)
void client_handler(char *request){
    char in_path[MAX_FILE_NAME_LEN];
    sprintf(in_path,"%s/%d",TEMP_DIR,getpid());
    mkfifo(in_path,0666);

    char server_path[MAX_FILE_NAME_LEN];
    sprintf(server_path,"%s/%s", TEMP_DIR, request);
    int out_fd = open(server_path, O_WRONLY, 0666);

    char msg[MAX_FILE_NAME_LEN];
    sprintf(msg,"%d",getpid());

    write(out_fd,msg,strlen(msg)+1);

    
    //printf("fd:%d",in_fd);
    //fflush(stdout);

    //int new_stdout = dup(STDOUT_FILENO);
    //dup2(out_fd, STDOUT_FILENO);
    //dup2(out_fd, STDERR_FILENO);

    int in_fd = open(in_path,O_RDONLY,0666);

    FILE* logf = openLogFile(request);

    
    close(STDOUT_FILENO);
    close(out_fd);
    close(STDERR_FILENO);
    

    

    // read(in_fd,msg,MAX_COMMAND_LENGTH);
    while (1)
    {
        // if(fcntl(out_fd, F_GETFD) == -1)
        //     out_fd = open(server_path, O_WRONLY, 0666);
        
        
        int read_len;
        if( (read_len = read(in_fd,msg,MAX_COMMAND_LENGTH)) == 0)
           break;
        //msg[read_len-2] = '\0';
        //write(new_stdout,msg,sizeof(msg));

        int out = open(server_path, O_WRONLY, 0666);
        command_handler(msg,myserver.directory, in_fd);
        
        // Log the request
        logMessage(logf,msg);

        
        // // Write end response
        // write(STDOUT_FILENO, EOF, sizeof(EOF) );

        close(out);
    }




}


// Process the request queue
// If any server children is available then redirect the request to that children
void process_queue(server *myserver){

    // If there is no any waiting client in the queue
    if(myserver->request_queue->size <= 0)
        return;

    if(myserver->available_child_size > 0)
    {
        for (int i = 0; i < myserver->max_client; i++)
        {
            if(myserver->children[i] == 0)
            {
                
                // Take a request from request queue
                char *request = dequeue(myserver->request_queue);

                if(request == NULL)
                    return;

                printf(">> Client:%s is popped from queue. \n", request);

                int pid = fork();

                switch (pid)
                {
                // Child process
                case 0:
                    is_parent = 0;
                    if(request[0] == 't')
                        client_handler(&request[1]);
                    else
                        client_handler(request);

                    exit(EXIT_SUCCESS);
                    break;

                // Error case
                case -1:
                    perror(">> Child Fork Error\n");
                    exit(EXIT_FAILURE);
                    break;

                // Parent process
                default:
                    myserver->children[i] = pid;
                    //return client_handler(request);
                    printf(">> Child:%d took client:%s \n", i, request);
                    // Free the request
                    free(request);
                    myserver->available_child_size--;
                    // Recursive call
                    return process_queue(myserver);
                }
            }


        }

    }


}













int main(int argc, char const *argv[])
{


    // printf("Number of arguments: %d\n", argc);

    // // Loop through each argument and print its value
    // for (int i = 0; i < argc; i++) {
    //     printf("Argument %d: %s\n", i, argv[i]);
    // }


    // if ( ! arg_check(argc, argv) )
    // {
    //     fprintf(stderr, "Wrong usage! Example Usage: server <dirname> <max. #ofClients>\n");
    //     return EXIT_FAILURE;
    // }

    int max_cli;
    char directory[MAX_COMMAND_LENGTH];

    printf("Enter Directory:");
    scanf("%s",directory);

    printf("Enter Max Client Size:");
    scanf("%d",&max_cli);



    // const char *arg_arr[] = {"name","/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/exampleDir","5"};
    // Takes the arguments and initialize the server struct and
    // all required initial operations (fifo, child processes array etc.)
    // init_server(&myserver,arg_arr);

   init_server(&myserver, max_cli, directory);


    char *request = NULL;

    while ( 1 )
    {
        // Sleep until a connection request and return the request
        request = get_request(&myserver);

        if(strlen(request) < 2)
            continue;

        printf(">> Request: %s\n",request);
        // Handle the request
        // If server children are busy add the request to the queue if client can wait
        handle_request(&myserver, request);

        // Process the request queue
        // If current server children size is not max then forks a children process for request
        process_queue(&myserver);


    }



    return 0;
}
