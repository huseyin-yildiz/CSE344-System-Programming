// Implementation of dup(oldfd) function
// Author : Mehmet Huseyin YILDIZ
// Date   : 26.02.2023

# include "dup.h"

int dup(int oldfd){

    extern int errno;
    int newfd;

    // Validation of old file descriptor
    if(fcntl(oldfd, F_GETFL) < 0)       // F_GETFL command returns the flags and access modes of the file description
    {
        errno = EBADF;
        return -1;
    }

    // Finds the lowest numbered fd greater than 0.
    newfd = fcntl(oldfd,F_DUPFD, 0);
    return newfd;
    

}


// // Tests
// int main(int argc, char const *argv[])
// {   
//     int result;

//     // Test Case 1 :Negative Invalid oldfd
//     result = dup(-5);
//     if(result == -1)
//         perror("FD_ERROR");
//     printf("Test with  -5 newfd: %d\n\n", result);
    
//     // Test Case 2 :Valid oldfd (0)
//     result = dup(0);
//     if(result == -1)
//         perror("FD_ERROR");
//     printf("Test with   0 newfd: %d\n\n", result);
    
//     // Test Case 3 :Valid oldfd (1)
//     result = dup(1);
//     if(result == -1)
//         perror("FD_ERROR");
//     printf("Test with   1 newfd: %d\n\n", result);
    
//     // Test Case 4 :Valid oldfd (2)
//     result = dup(2);
//     if(result == -1)
//         perror("FD_ERROR");
//     printf("Test with   2 newfd: %d\n\n", result);
    
//     // Test Case 5 :Positive invalid oldfd
//     result = dup(100);
//     if(result == -1)
//         perror("FD_ERROR");
//     printf("Test with 100 newfd: %d\n\n", result);


//     return 0;
// }