# include "dupped_fd_verifier.h"

void test_duplicated(int fd, int new_fd) {
    off_t offset1, offset2;

    char buf1[] = "Hello, world!";
    char buf2[] = "Goodbye, world!";


    // Get the old file descriptor offset
    offset1 = lseek(fd, 0, SEEK_CUR);
    printf("Old file descriptor offset: %ld\n", (long)offset1);


    // Verify that the file descriptors share the same file descriptor offset
    offset2 = lseek(new_fd, 0, SEEK_CUR);
    printf("New file descriptor offset: %ld\n", (long)offset2);


    // Write to the both file descriptors
    write(fd, buf1, sizeof(buf1)-1);
    write(new_fd, buf2, sizeof(buf2)-1);
    printf("Writing \"%s\" to the old fd\n", buf1);
    printf("Writing \"%s\" to the new fd\n", buf2);
    

    // Verify that the new file descriptor shares the same file descriptor offset
    offset1 = lseek(fd, 0, SEEK_CUR);
    offset2 = lseek(new_fd, 0, SEEK_CUR);
    printf("Old fd offset: %ld\n", (long)offset1);
    printf("New fd offset: %ld\n", (long)offset2);

    if (offset1 == offset2) {
        printf("Duplication successful: file descriptor offset is shared.\n");
    } else {
        printf("Duplication failed: file descriptor offset is not shared.\n");
    }


    // Close the file descriptors
    close(fd);
    close(new_fd);

    // Read the file
    printf("Closing both fd and reading the test file:\n");
    readFile("file.txt");

}



void readFile(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFSIZ];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, BUFSIZ)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
    printf("\n");
}




int main() {
    
    
    // fd verifying for dup
    printf("=================== Testing dup ===================\n");
    
    int fd = open("file.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    int newfd = dup(fd);
    test_duplicated(fd, newfd);



    // fd verifying for dup2
    printf("\n\n\n=================== Testing dup2 ===================\n");

    int fd1 = open("file.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    dup2(fd1, newfd);
    test_duplicated(fd, newfd);

    return 0;
}





