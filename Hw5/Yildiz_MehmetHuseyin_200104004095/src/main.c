# include "pcp.h"


queue buffer_queue;

// Atomic counters
_Atomic int done = 0;
_Atomic int dir_count = 0;
_Atomic int fifo_count = 0;
_Atomic int regular_count = 0;
_Atomic unsigned long long int total_bytes_count = 0;


int main(int argc, char const *argv[])
{

    if(argc != 5)
    {
        printf("Wrong usage\nExample usage: pCp <buffer_size> <num_of_consumers> <src_path> <dest_path> \n");
        exit(EXIT_FAILURE);
    }

    int buffer_size = atoi(argv[1]);
    int num_of_consumers = atoi(argv[2]);
    const char *src_path = argv[3];
    const char *dest_path = argv[4];
    
    // printf("%s %s",src_path, dest_path);

    init_queue(&buffer_queue, buffer_size);
    
    pthread_t **consumer_pool = calloc(sizeof(pthread_t*), num_of_consumers);

    pthread_t thread_produce;

    struct timeval start_time, end_time;
    double total_time;
    
    // Record the starting time
    gettimeofday(&start_time, NULL);  

    
    paths paths1;
    paths1.dest_path = (char*) dest_path;
    paths1.src_path = (char*) src_path;

    pthread_create(&thread_produce, NULL, produce, &paths1);
    
    // Create consumer pool threads
    for (int i = 0; i < num_of_consumers; i++)
    {
        consumer_pool[i] = malloc(sizeof(pthread_t));
        pthread_create(consumer_pool[i], NULL, consume, NULL);
    }
    
    // Wait for producer
    pthread_join(thread_produce, NULL);
    fflush(stdout);
    // printf("Producer joined \n");

    // Producing is end so unblock all waiting consumers
    unblock_queue(&buffer_queue);

    // Wait all consumers to join
    for (int i = 0; i < num_of_consumers; i++)
    {
        pthread_join(*consumer_pool[i], NULL);
        free(consumer_pool[i]);
    }

    free(consumer_pool);
    
    destruct_queue(&buffer_queue);

    printf("\nOperation done \n\n");
    
    // Record the ending time
    gettimeofday(&end_time, NULL);  
    
    // Calculate the total time in milliseconds
    total_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec) / 1000.0;  

    printf("Total elapsed time           : %.2f ms \n", total_time);
    printf("Total copied fifo files      : %d \n", fifo_count);
    printf("Total copied regular files   : %d \n", regular_count);
    printf("Total copied sub-directories : %d \n", dir_count);
    printf("Total copied number of bytes : %lld \n", total_bytes_count);
    
    return 0;
}



void *consume(){
    
    buffer_entry *entry = NULL;

    while (1)
    {
        // Fetch operation
        entry = dequeue(&buffer_queue);

        if(entry == NULL)
            break;
        
        char buffer[CP_BUFFER_SIZE];
        int read_size;
    
        // Doing copy operation
        while ( (read_size = read(entry->src_fd, buffer, CP_BUFFER_SIZE)) > 0 )
        {
            write(entry->dest_fd, buffer, read_size);
            
            // Increment the total_byte count
            atomic_fetch_add(&total_bytes_count,read_size);
        }

        // Copy operation done 

        printf("Regular File: %s done \n",entry->name);
        
        // Clear sources
        close(entry->src_fd);
        close(entry->dest_fd);
        free(entry);   
    }

}




// Create directory recursively
void make_dir(char *dir){

    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);    
}

// Add the file operation to the buffer 
void add_buffer(char *src_path, char *dest_path, char *file_name){

    buffer_entry *entry = malloc(sizeof(buffer_entry));
    
    strcpy(entry->name, file_name);
    entry->src_fd = open(src_path, O_RDONLY);
    entry->dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    enqueue(&buffer_queue, entry);
}


void* produce(void* paths1){
    
    paths * path = (paths*) paths1;
    char *src_path = path->src_path;
    char *dest_path = path->dest_path;

    // Create the destination directory
    make_dir(dest_path);

    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    // Open the directory
    dir = opendir(src_path);
    if (dir == NULL) {
        perror("Error opening directory");
        return NULL;
    }

    // Read directory entries one by one
    while ((entry = readdir(dir)) != NULL) {
        
        // Construct the file's full path
        char src_file_path[PATH_MAX];
        char dest_file_path[PATH_MAX];
        sprintf(src_file_path, "%s/%s", src_path, entry->d_name);
        sprintf(dest_file_path, "%s/%s", dest_path, entry->d_name);

        // Get file information
        if (stat(src_file_path, &fileStat) < 0) {
            perror("Error getting file information");
            continue;
        }

        // Check if it's a regular file
        if (S_ISREG(fileStat.st_mode)) {
            // printf("File: %s\n", entry->d_name);

            // Increment the regular file count
            atomic_fetch_add(&regular_count,1);

            // Add the file to the buffer
            add_buffer(src_file_path, dest_file_path, entry->d_name);

        }
        // Check if it's a directory
        else if (S_ISDIR(fileStat.st_mode)) {
            // Skip "." and ".." directories
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                
                // Increment the directory count
                atomic_fetch_add(&dir_count,1);

                // Recursive call to the directory
                paths path1 = {.dest_path = dest_file_path, .src_path = src_file_path };
                produce(&path1);

                printf("Directory: %s done\n", entry->d_name);
            }
        }

        // Check if it's a fifo
        else if (S_ISFIFO(fileStat.st_mode)) {

                // Increment the fifo count
                atomic_fetch_add(&fifo_count,1);

            mkfifo(dest_file_path, 0666);

            printf("Fifo: %s done\n", entry->d_name);   

        }

    }

    closedir(dir);


}
