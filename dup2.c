#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int dup2(int oldfd, int newfd){

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
    
	
    if (result < 0) 
		return result;
	
    else if (result != newfd) {
		close(result); /* this is not the newfd we are looking for */
		return -1;
	}
    else 
		return result;
	
    
}


// tests
int main(int argc, char const *argv[])
{
    
    dup2(0,0);
    dup2(0,3);
    dup2(0,1);
    dup2(100,105);
    dup2(100,1);
    dup2(1,110);

    return 0;
}

