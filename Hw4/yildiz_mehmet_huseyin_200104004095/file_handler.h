#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "file_sync.h"
#include <stdlib.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>


# define TEMP_DIR "/tmp/biboServer"  // Template directory
# define MAX_ARGS 32
# define BUFFER_SIZE 1024
# define MAX_COMMAND_LENGTH 1024

int terminate = 0;

int split_cmd(char *args[MAX_ARGS], char *command){

    int i = 0;
    // Extract the first token
    char * token = strtok(command, " ");
    // loop through the string to extract all other tokens
    while( token != NULL ) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    return i;

}


void print_errno_message(int fd, const char* message) {
    char buffer[256];

    int errnum = errno;  // Store the current errno value
    strerror_r(errnum, buffer, sizeof(buffer));

    // Create the error message with the custom message and errno string
    size_t message_len = strlen(message);
    size_t buffer_len = strlen(buffer);
    size_t total_len = message_len + buffer_len + 4;  // +4 for ": ", "\n", and null terminator
    char* error_message = (char*)malloc(total_len);
    snprintf(error_message, total_len, "%s: %s\n", message, buffer);

    // Write the error message to the specified file descriptor
    ssize_t bytes_written = write(fd, error_message, total_len - 1);
    if (bytes_written == -1) {
        perror("Error while writing to file descriptor");
    }

    free(error_message);
}


void help(char * cmd, int out_fd){

    if(cmd == NULL)
    {
        char msg[] =    "Available commands:\n- help\n- list\n- readF <file> <line #>\n- writeT <file> <line #> <string>\n- upload <file>\n- download <file>\n- quit\n- killServer\n";

        write(out_fd, msg, strlen(msg));

    }

    // TODO

}

char* path_merge(char *dir, char* file_name){
    strcat(dir,"/");
    strcat(dir,file_name);
    return dir;
}

void list(char* directory, int out_fd) {    
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            write(out_fd, dir->d_name, strlen(dir->d_name));
            write(out_fd, "\n", 1);
        }
        closedir(d);
    }


}






off_t seek_to_line(int fd, int line_number, int out_fd) {
    off_t offset = 0;
    char ch;
    ssize_t bytes_read;
    int current_line = 1;

    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n') {
            current_line++;
            if (current_line == line_number) {
                return offset + 1;  // Return offset after the newline character
            }
        }

        offset++;
    }

    if (bytes_read == -1) {
        print_errno_message(out_fd, "Error while reading file");
    }

    return -1;  // Line number not found
}



void print_until_newline(int fd, int out_fd) {
    char ch;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        write(out_fd, &ch, 1);
        if (ch == '\n') {
            break;  // Reached newline character
        }
    }

    if (bytes_read == -1) {
        print_errno_message(out_fd, "Error while reading file");
    }
}



void readF(char *filepath, int line, int out_fd){
    
    int fd = open(filepath, O_RDONLY, 0666);
    if(fd == -1) {
        
        print_errno_message(out_fd, "readF error:");
        return;
    }

    // Print all file
    if(line == 0)
    {
        
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            ssize_t bytes_written = write(out_fd, buffer, bytes_read);
            if (bytes_written == -1) {
                
                print_errno_message(out_fd, "Error while writing to stdout" );
                return;
            }
        }

        if (bytes_read == -1) {
            print_errno_message(out_fd, "Error while reading file");
        }
        
    
    }
    // If the line num is not 1 then seeks to the given line
    else if (line != 1)
        if(seek_to_line(fd, line, out_fd) == -1)
        {
            char msg[MAX_COMMAND_LENGTH];
            sprintf(msg, "readF: Line number %d not found ",line);
            write(out_fd,msg,strlen(msg));
            return;
        }
    // Prints until newline char
    print_until_newline(fd, out_fd);
    
}




void writeT(char* filepath, int line_num, char* text, int out_fd) {
    
    // If line num is not given then appends
    if(line_num == 0)
    {
        FILE* file = NULL;
        file = fopen(filepath, "a");
        fprintf(file,"%s",text);
        fclose(file);
        return;
    }


    FILE* original_file = fopen(filepath, "r");
    
    // If file is not exist
    if (original_file == NULL) {
        
        FILE* file = fopen(filepath, "w");
        fprintf(file,"%s",text);
        fclose(file);
        return;
    
    }
    
    FILE* temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL) {
        char msg[] = "Failed to create temporary file.\n";
        write(out_fd, msg, strlen(msg));
        fclose(original_file);
        return;
    }
    
    char buffer[1024];
    int current_line = 1;
    while (fgets(buffer, sizeof(buffer), original_file)) {
        if (current_line == line_num) {
            fprintf(temp_file, "%s\n", text);
        } else {
            fprintf(temp_file, "%s", buffer);
        }
        current_line++;
    }
    
    fclose(original_file);
    fclose(temp_file);
    
    // Replace the original file with the modified temporary file
    remove(filepath);
    rename("temp.txt", filepath);


}



// reads and saves the file
void upload(char *directory, char *filename, int out_fd, int client_pid){
    
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


    char path[MAX_PATH_LENGTH];
    sprintf(path, "%s/%s",directory, newFilename);
    
    while (access(path, F_OK) != -1) {
        suffix++;
        sprintf(newFilename, "%s%d%s", filename, suffix, extension);
        sprintf(path, "%s/%s",directory, newFilename);
    }

    
    // lock the file with named binary semaphore (mutex)
    wait_file_line(newFilename,0);

        


    // Open the file for writing
    int file = open(path, O_WRONLY | O_CREAT| O_TRUNC, S_IRUSR | S_IWUSR);
    if (file == -1) {
        print_errno_message(out_fd, "Failed to open file");
        return;
    }

    // Read from the file descriptor and write to the file
    char buffer[MAX_COMMAND_LENGTH];
    ssize_t bytesRead;
    ssize_t bytesWritten;
    
    char file_path[MAX_PATH_LENGTH];

    sprintf(file_path, "%s/%d.d",TEMP_DIR, client_pid);
    int in_fd = open(file_path, O_RDONLY);

    while ((bytesRead = read(in_fd, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(file, buffer, bytesRead);
        if(bytesRead != sizeof(buffer))
            break;
    }

    // Close the file
    close(file);
    close(in_fd);
    unlink(file_path);

    post_file_line(newFilename,0);
    char msg[] = "Upload Completed\n";
    write(out_fd, msg, strlen(msg));

}


// send file to client
void download(char *filename, int out_fd){

    
    // char upload_path[MAX_COMMAND_LENGTH];
    
    usleep(100 * 1000);
    // Open the file for reading
    int file = open(filename, O_RDONLY);
    if (file == -1) {
        print_errno_message(out_fd, "Download");
        return;
    }

    // Read and write the file content
    char buffer[MAX_COMMAND_LENGTH];
    ssize_t bytesRead;
    ssize_t bytesWritten;

    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(out_fd, buffer, bytesRead);
        if (bytesWritten == -1) {
            print_errno_message(out_fd, "Failed to write to file descriptor");
            break;
        }
    }

    // Close the file
    close(file);
    
    close(out_fd);

    fflush(stdout);  
    
}




void quit(){

}

void killServer(){

}





















// It will take a command and call the command function that will write the result to the given fd
void command_handler(char* command, char* directory, int out_fd, int client_pid ){

    char* args[MAX_ARGS] = {0};
    char copy_cmd[MAX_COMMAND_LENGTH];
    char copy_directory[MAX_COMMAND_LENGTH];

    strcpy(copy_cmd, command);
    strcpy(copy_directory, directory);

    int arg_size = split_cmd(args, command);
    char *file_path;
    

    if (strncmp(command, "help", 4) == 0) {
        
        help(args[1], out_fd);
        

    } else if (strncmp(command, "list", 4) == 0) {
        list(copy_directory, out_fd);
        
    } else if (strncmp(command, "readF", 5) == 0) {
        
        int line;
        file_path = path_merge(copy_directory, args[1]);
        
        if(args[2] != NULL)
            line = atoi(args[2]);
        else
            line = 0;
        
        // lock the file with named binary semaphore (mutex)
        wait_file_line(args[1],0);

        readF(file_path, line, out_fd);

        post_file_line(args[1],0);


        
    } else if (strncmp(command, "writeT", 6) == 0) {
        int line;
        file_path = path_merge(copy_directory, args[1]);
        char * text;

        if(isdigit( args[2][0] ) )
            {
                line = atoi(args[2]);
                int index = args[3] - args[0];
                text = &copy_cmd[index];
            }
        else
            {
                line = 0;
                int index = args[2] - args[0];
                text = &copy_cmd[index];
            }
        
        // lock the file with named binary semaphore (mutex)
        wait_file_line(args[1],0);

        writeT(file_path, line, text, out_fd);

        post_file_line(args[1],0);



    } else if (strncmp(command, "upload", 6) == 0) {
        
        
        upload(directory,args[1], out_fd, client_pid );
        
        
        
    } else if (strncmp(command, "download", 8) == 0) {
        
        // lock the file with named binary semaphore (mutex)
        

        char path[MAX_PATH_LENGTH];
        sprintf(path, "%s/%s",directory, args[1]);

        wait_file_line(args[1],0);
        
        download(path, out_fd);

        post_file_line(args[1],0);


    } else if (strcmp(command, "quit") == 0) {
        printf(">> Client%d disconnected. \n", client_pid);

    } else if (strcmp(command, "killServer") == 0) {
        char msg[] = "Sending a kill request to the Server\n";
        write(out_fd, msg, strlen(msg));
        printf(">> kill signal from Client%d.. ",client_pid);
        terminate = 1;
        
    } else {
        char msg[MAX_ARGS];
        sprintf(msg,"Invalid command:%s \n",copy_cmd);
        write(out_fd, msg, strlen(msg));
    }






}



// int main(int argc, char const *argv[])
// {
//     char text[] = "help a w r 2";
//     //command_handler(STDOUT_FILENO, text, "/temp/" );
    
//     help(NULL);

//     printf("\n ----------- list -----------");
//     fflush(stdout);
    
//     list("/");
    

//     printf("\n ----------- readF -----------");
//     fflush(stdout);
    
//     readF("/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/test.txt",1);
//     readF("/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/test.txt",2);
//     readF("/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/test.txt",5);
//     readF("/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/test.txt",10);
//     readF("/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/test.txt",0);
    

//     printf("\n ----------- writeT -----------");
//     fflush(stdout);

//     char dir[] = "/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/test.txt";

//     // command_handler("",dir);

//     writeT("/home/huseyin/Desktop/System Programming CSE344/Homeworks/CSE344-System-Programming/Midterm/exampleDir/a",5,"asdasdadasdas");
    

    
//     return 0;
// }



