#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>



int run_command(char* command, int input_fd, int output_fd);
void redirect_io(char* command);
int is_free(char* cmd);
int open_log_file();
void handle_sigint(int signum);


#define MAX_CMD_LEN 1024
#define MAX_FILE_NAME_LEN 255
#define LOG_DIRECTORY "./logs/"

const char pip[2] = "|";
pid_t children[20];
int proc_count;                   // Process count
char input_str[MAX_CMD_LEN];
int status;
char buffer[1000];

int main()
{

  char *command;                    // Command that took from user
  char *context_pip;                // strtok context for pip splitting
  int pipes[20][2];                 // To hold all pipes in an array
  char command_copy[MAX_CMD_LEN];   // copy of command to send to child process

  // Shell loop
  while (1)
  {
    proc_count = 0; 
    
    // Prompt message
    printf("₺ ");

    // Flush the prompt to see the message correctly
    fflush(stdout);
    

    // Get command from user
    if (fgets(input_str, MAX_CMD_LEN, stdin) == NULL)
      break;

    // Quit command check
    if( strcmp(input_str, ":q\n") == 0 )
        exit(EXIT_SUCCESS);

    // Handle free command
    if (is_free(input_str))
      continue;

    // Open a log file with timestamp
    int log_fd = open_log_file();
    

    // Signal Handler setup
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&(sa.sa_mask));
    sigaddset(&(sa.sa_mask), SIGINT);
    sigaction(SIGINT, &sa, NULL);


    // Split the command with pipe operator
    command = strtok_r(input_str, pip, &context_pip);

    pipes[0][0] = STDIN_FILENO;

    // It process commands after every pipe
    while (command != NULL )
    {
      proc_count++;
      pipe( pipes[proc_count] );
      
      // Copy of the command to send to child
      strcpy(command_copy, command);

      pid_t child_pid;

      // Split the command with pipe again
      command = strtok_r(NULL, pip, &context_pip);

      // If the command is last command then we make it's output STDOUT
      // Runs the command with a new child process and gives it's input and outputs 
      if(command == NULL)
        child_pid = run_command(command_copy, pipes[proc_count-1][0], STDOUT_FILENO);
      
      // If it's not las command then we connect it's output to the next child's input with a pipe
      else
        child_pid = run_command(command_copy, pipes[proc_count-1][0], pipes[proc_count][1]);

      children[proc_count-1] = child_pid;

      // Write log entry for every child processes
      char log_entry[MAX_CMD_LEN+10];
      sprintf(log_entry, "%d : %s \n", child_pid, command_copy);
      write(log_fd, log_entry, strlen(log_entry));

      // Close the write end of pipe of parent.
      close(pipes[proc_count][1]); 
      
    } // End of one line command processing


    // Close log file
    close(log_fd);

    // Wait the child processes and handle the status of signals
    // If child processes terminated not normally then it prints and returns shell back
    for (int i = 0; i < proc_count; i++)
    { 
      int status;
      pid_t pid = wait(&status);
      
      if (WIFEXITED(status)) 
        printf("Child process %d exited with status %d\n", pid, WEXITSTATUS(status));
      else if (WIFSIGNALED(status)) 
        printf("Child process %d terminated by signal %d\n", pid, WTERMSIG(status));
      else 
        printf("Child process %d terminated abnormally\n", pid);
    
    }
    
  }



  return 0;
}


// Takes the splitted command and takes input and output fd
// Process the command for < and >
pid_t run_command(char *command, int input_fd, int output_fd)
{
  int status;
  pid_t childPid;
  char cmd_copy[MAX_CMD_LEN];

  switch (childPid = fork())
  {
    case -1:          /* process creation error */
      return -1;
    
    case 0:       /* Child */
      printf("PID:%8d PPID:%8d\n",getpid(), getppid());
      fflush(stdout);

      // Change the input and output to the given fd's      
      dup2(input_fd, STDIN_FILENO);
      dup2(output_fd, STDOUT_FILENO);


      // !!!! close other end of pipe 
      
      // Divide the command 2 part (basic command and redirection part)
      char *cmd_part1, *cmd_part2, *context2;

      strcpy(cmd_copy,command);

      // Extract command string
      cmd_part1 = strtok_r(command, "<>", &context2);
      printf("%s",cmd_part1);

      // Extract redirection string that starts with < or >
      cmd_part2 = &cmd_copy[ strlen(cmd_part1) ];
      
      // Redirect IO according to the given redirection string. It parses and redirects the input and output.
      redirect_io(cmd_part2);
      
      // Run the basic command (without pipe and redirection operators)
      execl("/bin/sh", "sh", "-c", cmd_part1, (char *)NULL);
      
      // Exec fail
      _exit(127);
      
  
    
    
      //default: /* Parent */
        // close(input_fd);
        // close(output_fd);
        
    //   if (waitpid(childPid, &status, 0) == -1)
    //     return -1;
    //   else
    //     return status;
  }

  return childPid;
}


// Take redirection part of command (like: < input.txt > output.txt) and change input and output
void redirect_io(char* command){
  
  char file_name[MAX_FILE_NAME_LEN];
  
  int len = strlen(command);
  
  for (int i = 0; i < len; i++)
  {
    // Redirection of input 
    if(command[i] == '<'){
      
      if(sscanf(command+i+1, "%s",file_name) == 1)
      {  
        
        // Open the input file in read mode
        int input_fd = open(file_name, O_RDONLY); 
        if(input_fd == -1){
          perror("");
          exit(EXIT_FAILURE);
        }

        else{
          // Redirect input_fd to STDIN_FILENO
          dup2(input_fd, STDIN_FILENO); 
          i += strlen(file_name);
        }

      }
      
      // Error message
      else{
          fprintf(stderr,"Wrong usage of <");
          exit(EXIT_FAILURE);
      }
    
    }


    // Redirection of output 
    if(command[i] == '>'){
      
      if(sscanf(command+i+1, "%s",file_name) == 1)
      {  
        
        // Open the output file in write mode
        int output_fd = open(file_name, O_WRONLY | O_CREAT ,0666); 
        if(output_fd == -1){
          perror("");
        }
        
        else{
          // Redirect output_fd to STDOUT_FILENO
          dup2(output_fd, STDOUT_FILENO); 
          i += strlen(file_name);
        }

      }

      // Error message
      else{
          fprintf(stderr,"Wrong usage of >");
          return(EXIT_FAILURE);
      }
    
    }




      
  }
  
}

// Check if the command is free (space or nothing) (like : "   \n", "\n" etc.)
int is_free(char* cmd){

    if(cmd[0] == '\n')
      return 1;

    int cmd_len = strlen(cmd), ind;
    for ( ind=0 ; ind < cmd_len ; ind++)
      if( ! isspace(cmd[ind]) )
          return 0;
    return 1;

}

// Opens a log file in log directory with current timestamp
int open_log_file(){
  
    // Check if directory exists
    if (access(LOG_DIRECTORY, F_OK) != 0) {
        // Create directory if it does not exist
        if (mkdir(LOG_DIRECTORY, 0777) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
        // printf("Directory created successfully.\n");
    } else {
        // printf("Directory already exists.\n");
    }

    // Get current time
    char file_name[100];
    time_t t = time(NULL);
    
    // Get timestamp string 
    char *time_stamp  = ctime(&t);
    time_stamp[strlen(time_stamp)-1] = '\0';
    
    // Format the name of the log file
    sprintf(file_name,"%s%s.log", LOG_DIRECTORY, time_stamp);

    // Open the file
    int fd = open(file_name, O_RDWR | O_CREAT ,0777);
    return fd;

}


// signal handler for SIGINT
void handle_sigint(int signum) {
  printf("Caught SIGINT signal. Sending to children...\n");
  
  for (int i = 0; i < proc_count; i++)
    kill(children[i], SIGINT); // send SIGINT signal to child process     

}