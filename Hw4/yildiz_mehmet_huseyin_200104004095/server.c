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
# include <pthread.h>



# define MAX_FILE_NAME_LEN 256
# define MAX_THREAD_SIZE 100




typedef struct
{
    int max_client;                 // Maximum simultaneous client capacity
    const char *directory;                // The directory server run on
    int connection_fd;              // File Descriptor (Fifo) for incominq requests
    char connection_path[MAX_FILE_NAME_LEN];
    sem_t *sem_connection;
    sem_t *write_mutex;


    int thread_size;
    pthread_t threads[MAX_THREAD_SIZE];
    queue *request_queue;           // Request queue
    



}server;


typedef struct
{
    pid_t pid;
    char* msg;

}request;



pthread_mutex_t queue_mutex;
sem_t queue_sem;

queue req_queue = { .front = 0, .rear = 0, .size = 0, .mutex = &queue_mutex, .semaphore = &queue_sem};
server myserver = {.request_queue = &req_queue };







// Signal handler function
void sigintHandler(int sig) {
    printf(">> Received SIGINT signal. \n");

    printf(">> Waiting all jobs to be done");
    
    int remaining_job;
    while(1)
    {
        printf(".");
        sem_getvalue( myserver.request_queue->semaphore, &remaining_job);
        if(remaining_job == 0)
            break;
        sleep(1);
    }

    printf("terminating...\n");

    
    // Cancel all waiting threads
    for (int i = 0; i < myserver.thread_size; i++)
        pthread_cancel(myserver.threads[i] );    
    

    

    exit(EXIT_SUCCESS);

}





// Func for threads
// pull a request from request queue and handle it
void handle_request(){

    char client_fifo_path[MAX_PATH_LENGTH];
    int out_fd;
    while (! terminate)
    {
        // printf("Thread:%lu is waiting for a request \n", (unsigned long)pthread_self());
        request* req = dequeue( myserver.request_queue );

        // TODO : process
        printf("The req:\"%s\" from pid:%d \n", req->msg, req->pid);
        
        fflush(stdout);
        sprintf(client_fifo_path,"%s/%d",TEMP_DIR, req->pid);
        out_fd = open(client_fifo_path, O_WRONLY);

        // printf("Fifo %s opened\n",client_fifo_path);

        // Process the request and send response
        command_handler(req->msg, myserver.directory, out_fd, req->pid );

        // Close the client (response) fd
        close(out_fd);

        // Free all used mem area for the processed request
        free(req->msg);
        free(req);
    }
    
    
}



// Initialize all server threads that handles requests
void init_threads(server *myserver){

    for (int i = 0; i < myserver->thread_size; i++)        
        if (pthread_create(&myserver->threads[i], NULL, handle_request, NULL) != 0) {
            fprintf(stderr, "Failed to create thread %d.\n", i);
            return 1;
        }

}





// Takes the arguments and initialize the server struct and all required initial operations (fifo, child processes etc.)
void init_server(server* myserver, int max_cli, char *directory, int thread_size){


    myserver->max_client = max_cli;
    myserver->directory  = directory;

    // Create a template directory for the this app
    mkdir(TEMP_DIR, 0777);

    sprintf(myserver->connection_path, "%s/%d", TEMP_DIR, (int)getpid());

    // Create fifo for incoming requests
    mkfifo(myserver->connection_path, 0666);

    char sem_name[MAX_FILE_NAME_LEN];
    char mutex_name[MAX_FILE_NAME_LEN];

    sprintf(sem_name, "%d.sem", (int)getpid() );
    sprintf(mutex_name, "%d.mtx", (int)getpid());
    
    // Create or open the named connection semaphore
    myserver->sem_connection = sem_open(sem_name, O_CREAT, 0666, myserver->max_client);
    
    if (myserver->sem_connection == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }


    // Create or open the named write mutex (binary semaphore)
    myserver->write_mutex = sem_open(mutex_name, O_CREAT, 0666, 1);
    
    if (myserver->write_mutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }



     // SIGINT Signal Handler setup
    struct sigaction sa1;
    sa1.sa_handler = sigintHandler;
    sigemptyset(&(sa1.sa_mask));
    sigaddset(&(sa1.sa_mask), SIGINT);
    sigaction(SIGINT, &sa1, NULL);






    printf(">> Server Started PID %d... \n",getpid());
    printf(">> waiting for clients...\n");

    myserver->connection_fd = open(myserver->connection_path, O_RDONLY , 0666);

    // Open a dummyfd to be blocked in every read
    int dummyFd = open(myserver->connection_path, O_WRONLY);
    if (dummyFd == -1)
        perror("Dummy fd error");


    myserver->thread_size = thread_size;



    


    // Initialize the request queue mutex
    if(pthread_mutex_init(myserver->request_queue->mutex, NULL))
        perror("pthread_mutex_init");
    
    if(sem_init(myserver->request_queue->semaphore, 0, 0) == -1)
        perror("sem_init");



}




// Read a request from server fifo
request * get_request(server *myserver){

    request *req = malloc(sizeof(request));

    int len;

    read(myserver->connection_fd, &req->pid, sizeof(pid_t) );
    read(myserver->connection_fd, &len, sizeof(int) );
    req->msg = malloc(len);
    read(myserver->connection_fd, req->msg, len);

    return req;
}









int main(int argc, char const *argv[])
{

    if(argc != 4)
    {
        printf("Usage: biboServer <dirname> <max.#ofClients> <poolSize> \n");
        return EXIT_FAILURE;
    }    

    request *req;

    int max_client = atoi(argv[2]);
    char *dir = argv[1];
    int thread_size = atoi(argv[3]);
    
    // int max_client = 2;
    // char *dir = "..";
    // int thread_size = 3;
    

    pid_t clients[MAX_THREAD_SIZE]; 
    int client_count = 0;

    // Initialize the server (server fifo, connection semaphore, mutexes, signal handling etc.)
    init_server(&myserver, max_client, dir, thread_size);

    FILE * logF = openLogFile();

    // Initialize all threads
    init_threads(&myserver);


    while (! terminate)
    {
        // Get request from server fifo
        req = get_request(&myserver);
        
        // Log the request
        char logmsg[MAX_COMMAND_LENGTH];
        sprintf(logmsg,"%d %s",req->pid, req->msg);
        logMessage(logF, logmsg);


        if(strncmp("killServer",req->msg,10) == 0 )
        {
            printf(">> kill signal from Client%d.. \n",req->pid);
            terminate = 1;
            break;
        }

        // add the request to the threadsafe queue
        enqueue( myserver.request_queue, req);

    }

    
    printf(">> Waiting all jobs to be done");
    
    int remaining_job;
    while(1)
    {
        printf(".");
        sem_getvalue( myserver.request_queue->semaphore, &remaining_job);
        if(remaining_job == 0)
            break;
        sleep(1);
    }

    printf("terminating...\n");

    
    // Cancel all waiting threads
    for (int i = 0; i < myserver.thread_size; i++)
        pthread_cancel(myserver.threads[i] );    
    



    return 0;
}
