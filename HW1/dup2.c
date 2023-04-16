#include "dup2.h"

int mydup2(int oldfd, int newfd){

    extern int errno;
    int result;

    // Validation of old file descriptor
    if(fcntl(oldfd, F_GETFL) < 0)       // F_GETFL command returns the flags and access modes of the file description
    {
        errno = EBADF;
        return -1;
    }

    
    // if the oldfd and newfd equal then just return the newfd
    if(oldfd == newfd)
        return newfd;


    // If the newfd is open then close it
    if( fcntl(newfd,F_GETFL) >= 0)
        close(newfd);


    result = fcntl(oldfd, F_DUPFD, newfd);
    
	// if the result is error, then return same result
  if (result < 0) 
	  return result;
	
  // If the result fd is not that we want, then we close it
  else if (result != newfd) {
		close(result); 
		return -1;
	}
  else    
		return result;
	
    
}



// Tests
int main(int argc, char const *argv[])
{   
    
    // Test Case 1: Valid oldfd and newfd descriptor
    
    int fd = open("file.txt", O_CREAT | O_WRONLY, 0666);
    int newfd = mydup2(fd, 5);
    // Verify that the return value of dup2 is equal to newfd
    assert(newfd == 5);
    // Verify that the file descriptors refer to the same file
    assert(fcntl(fd, F_GETFL) == fcntl(newfd, F_GETFL));
    close(fd);



    // Test Case 2: Invalid oldfd
    
    newfd = mydup2(100, 5);
    // Verify that the return value of dup2 is -1
    assert(newfd == -1);
    // Verify that errno is set to EBADF
    assert(errno == EBADF);

   
   
    // Test Case 3: Special case where oldfd equals newfd and oldfd is valid
   
    fd = open("file.txt", O_CREAT | O_WRONLY, 0666);
    newfd = mydup2(fd, fd);
    // Verify that the return value of dup2 is equal to newfd
    assert(newfd == fd);
    // Verify that the file descriptors refer to the same file
    assert(fcntl(fd, F_GETFL) == fcntl(newfd, F_GETFL));
    close(fd);

   

    // Test Case 4: Special case where oldfd equals newfd and oldfd is invalid
    
    newfd = mydup2(100, 100);
    // Verify that the return value of dup2 is -1
    assert(newfd == -1);
    // Verify that errno is set to EBADF
    assert(errno == EBADF);
    
    
    
    // Test Case 5: Multiple calls to dup2 with same oldfd and different newfd
    
    fd = open("file.txt", O_CREAT | O_WRONLY, 0666);
    int newfd1 = mydup2(fd, 5);
    int newfd2 = mydup2(fd, 6);
    // Verify that the return value of dup2 is equal to newfd
    assert(newfd1 == 5 && newfd2 == 6);
    // Verify that the file descriptors refer to the same file
    assert(fcntl(fd, F_GETFL) == fcntl(newfd1, F_GETFL));
    assert(fcntl(fd, F_GETFL) == fcntl(newfd2, F_GETFL));
    // Verify that newfd1 and newfd2 are different
    assert(newfd1 != newfd2);


    // If there is no any error until here then the program can print this
    printf("All cases tested with asserts. No any error in all cases \n");

    return 0;
}