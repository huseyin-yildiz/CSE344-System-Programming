#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

// The byte to write to the file
const char BYTE_TO_WRITE = 0b01010101;


int main(int argc, char const *argv[])
{

    
    int flags = O_CREAT | O_WRONLY;
    int fd;
    int mode = 0666;
    const char* file_name;
    bool x_param = false;
    int num_bytes;


    // Parameter Checking 

    if(argc == 3)
        flags |= O_APPEND;
    
    else if(argc == 4)
        x_param = true; 
    else
    {
        fprintf(stderr,"Usage: ./appendMeMore filename num-bytes [x] \n");
        exit(EXIT_FAILURE);
    }

    file_name = argv[1];
    int valid = sscanf(argv[2],"%d",&num_bytes);
    if(valid != 1){
        fprintf(stderr, "The num_bytes must be an integer");
        exit(EXIT_FAILURE);
    }


    // Opening File
    fd = open(file_name, flags, mode);
        if( fd == -1 ){
            perror("File error");
            exit(EXIT_FAILURE);
        }

    
    
    
    
    // Writing the BYTE_TO_WRITE value to the file 
    for (int i = 0; i < num_bytes; i++)
    {
        // Checking parameter x for lseek
        if( x_param )
            lseek(fd, 0, SEEK_END);
        

        if( write(fd, &BYTE_TO_WRITE, 1) != 1)
        {
            perror("Write Error");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }
    

    
    // Closing the file
    close(fd);
    
    
    return 0;
}
