#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

#define MAX_PATH_LENGTH 256

#define MAX_SAME_FILE_ACCESS 5

void do_secure_operation(const char* fileName, int lineNumber) {
    // Create/open the main door semaphore
    char mainDoorName[MAX_PATH_LENGTH];
    snprintf(mainDoorName, MAX_PATH_LENGTH, "/main_door_%s", fileName);
    sem_t* mainDoor = sem_open(mainDoorName, O_CREAT, 0666, 1);
    if (mainDoor == SEM_FAILED) {
        perror("Failed to create/open main door semaphore");
        return;
    }

    // Create/open the full line semaphore
    char fullLineName[MAX_PATH_LENGTH];
    snprintf(fullLineName, MAX_PATH_LENGTH, "/full_line_%s", fileName);
    sem_t* fullLine = sem_open(fullLineName, O_CREAT, 0666, MAX_SAME_FILE_ACCESS);
    if (fullLine == SEM_FAILED) {
        perror("Failed to create/open full line semaphore");
        sem_close(mainDoor);
        return;
    }

    sem_wait(mainDoor);

    if (lineNumber != -1) {
        // Create/open the line semaphore
        char lineMutexName[MAX_PATH_LENGTH];
        snprintf(lineMutexName, MAX_PATH_LENGTH, "/line_%s_%d", fileName, lineNumber);
        sem_t* lineSemaphore = sem_open(lineMutexName, O_CREAT, 0666, 1);
        if (lineSemaphore == SEM_FAILED) {
            perror("Failed to create/open line semaphore");
            sem_post(mainDoor);
            sem_close(fullLine);
            sem_close(mainDoor);
            return;
        }

        
        sem_wait(fullLine);
        sem_wait(lineSemaphore);

        sem_post(mainDoor);

        // Perform operations
        printf("Performing operations on line %d of file %s\n", lineNumber, fileName);
        fflush(stdout);
        sleep(5);
        
        sem_post(lineSemaphore);
        sem_post(fullLine);
        
        sem_close(lineSemaphore);
        
        printf("Performing operations on line %d of file done %s\n", lineNumber, fileName);
        fflush(stdout);
    } else {
        int val;
        sem_getvalue(fullLine, &val);
        for (int i=0; i<MAX_SAME_FILE_ACCESS; i++){
            sem_wait(fullLine);
            sem_getvalue(fullLine, &val);
        }
        // Perform operations
        printf("Performing operations on the entire file %s\n", fileName);
        fflush(stdout);
        sleep(4);

        sem_getvalue(fullLine, &val);
        for (int i=0; i<MAX_SAME_FILE_ACCESS; i++)
        {            
            sem_post(fullLine);
            sem_getvalue(fullLine, &val);
        }
        sem_post(mainDoor);
        printf("Performing operations on the entire file done %s\n", fileName);
    }

    sem_close(fullLine);
    sem_close(mainDoor);
    
    fflush(stdout);
    // sem_unlink(fullLineName);
    // sem_unlink(mainDoorName);
}

















void wait_file_line(const char* fileName, int lineNumber) {
    // Create/open the main door semaphore
    char mainDoorName[MAX_PATH_LENGTH];
    snprintf(mainDoorName, MAX_PATH_LENGTH, "/main_door_%s", fileName);
    sem_t* mainDoor = sem_open(mainDoorName, O_CREAT, 0666, 1);
    if (mainDoor == SEM_FAILED) {
        perror("Failed to create/open main door semaphore");
        return;
    }

    // Create/open the full line semaphore
    char fullLineName[MAX_PATH_LENGTH];
    snprintf(fullLineName, MAX_PATH_LENGTH, "/full_line_%s", fileName);
    sem_t* fullLine = sem_open(fullLineName, O_CREAT, 0666, MAX_SAME_FILE_ACCESS);
    if (fullLine == SEM_FAILED) {
        perror("Failed to create/open full line semaphore");
        sem_close(mainDoor);
        return;
    }

    sem_wait(mainDoor);

    if (lineNumber != -1) {
        // Create/open the line semaphore
        char lineMutexName[MAX_PATH_LENGTH];
        snprintf(lineMutexName, MAX_PATH_LENGTH, "/line_%s_%d", fileName, lineNumber);
        sem_t* lineSemaphore = sem_open(lineMutexName, O_CREAT, 0666, 1);
        if (lineSemaphore == SEM_FAILED) {
            perror("Failed to create/open line semaphore");
            sem_post(mainDoor);
            sem_close(fullLine);
            sem_close(mainDoor);
            return;
        }

        
        sem_wait(fullLine);
        sem_wait(lineSemaphore);

        sem_post(mainDoor);

        
    } else {
        int val;
        sem_getvalue(fullLine, &val);
        for (int i=0; i<MAX_SAME_FILE_ACCESS; i++){
            sem_wait(fullLine);
            sem_getvalue(fullLine, &val);
        }

        
    }

}



























void post_file_line(const char* fileName, int lineNumber) {
    // Create/open the main door semaphore
    char mainDoorName[MAX_PATH_LENGTH];
    snprintf(mainDoorName, MAX_PATH_LENGTH, "/main_door_%s", fileName);
    sem_t* mainDoor = sem_open(mainDoorName, O_CREAT, 0666, 1);
    if (mainDoor == SEM_FAILED) {
        perror("Failed to create/open main door semaphore");
        return;
    }

    // Create/open the full line semaphore
    char fullLineName[MAX_PATH_LENGTH];
    snprintf(fullLineName, MAX_PATH_LENGTH, "/full_line_%s", fileName);
    sem_t* fullLine = sem_open(fullLineName, O_CREAT, 0666, MAX_SAME_FILE_ACCESS);
    if (fullLine == SEM_FAILED) {
        perror("Failed to create/open full line semaphore");
        sem_close(mainDoor);
        return;
    }

    
    if (lineNumber != -1) {
        // Create/open the line semaphore
        char lineMutexName[MAX_PATH_LENGTH];
        snprintf(lineMutexName, MAX_PATH_LENGTH, "/line_%s_%d", fileName, lineNumber);
        sem_t* lineSemaphore = sem_open(lineMutexName, O_CREAT, 0666, 1);
        if (lineSemaphore == SEM_FAILED) {
            perror("Failed to create/open line semaphore");
            return;
        }

        
        
        sem_post(lineSemaphore);
        sem_post(fullLine);
        
        sem_close(lineSemaphore);
        
        
    } else {
        int val;

        sem_getvalue(fullLine, &val);
        for (int i=0; i<MAX_SAME_FILE_ACCESS; i++)
        {            
            sem_post(fullLine);
            sem_getvalue(fullLine, &val);
        }
        sem_post(mainDoor);
        
    }

    sem_close(fullLine);
    sem_close(mainDoor);
    
    // sem_unlink(fullLineName);
    // sem_unlink(mainDoorName);
}









int main(int argc, char const *argv[])
{
    
    

    if(fork() != 0){

        wait_file_line("example19.txt",-1);

        // Perform operations
        printf("line 4 proc \n");
        fflush(stdout);
        sleep(4);
        printf("line 4 proc done\n");
        fflush(stdout);
        post_file_line("example19.txt",-1);



            
        wait(NULL);
        

    }

    else{
        

        wait_file_line("example19.txt",4);

        // Perform operations
        printf("line 4 proc \n");
        fflush(stdout);
        sleep(4);
        printf("line 4 proc done\n");
        fflush(stdout);

        post_file_line("example19.txt",4);


          
        
        
        wait(NULL);
    
    
    }

    return 0;
}





















// // Tests

// int main(int argc, char const *argv[])
// {
    
    

//     if(fork() != 0){


//             do_secure_operation("example16.txt", -1);
//             //printf("line 1 ok\n");
//             fflush(stdout);

//             do_secure_operation("example14.txt", 3);
//             //printf("line 1 ok\n");
//             fflush(stdout);


//             do_secure_operation("example14.txt", -1);
//             //printf("all 1 ok\n");
//             fflush(stdout);
            
//             do_secure_operation("example14.txt", -1);
//            // printf("all 2 ok\n");
//             fflush(stdout);
            
            
//             wait(NULL);
        

//     }

//     else{
        
//             do_secure_operation("example15.txt", -1);
//             //printf("line 1 ok\n");
//             fflush(stdout);
        
//             do_secure_operation("example14.txt", 2);
//             // printf("line 2 ok\n");
//             fflush(stdout);
        
//             do_secure_operation("example14.txt", 3);
//             // printf("line 3 ok\n");
//             fflush(stdout);
        

//             do_secure_operation("example14.txt", 4);
//             // printf("line 4 ok\n");
//             fflush(stdout);
        
        
        
//         wait(NULL);
    
    
//     }

//     return 0;
// }


// // void do_secure_operation(file_name , line_num){

// //     main_door = sem_open(file_name, value = 1)
// //     full_line = sem_open(file_name+"fulls" , value = MAX_SAME_FILE_ACCESS)


// //     sem_wait(main_door)

// //     if(line != -1){

// //         line_s = sem_open(file_name+line_num, value = 1) 
// //         sem_post(main_door)
// //         sem_wait(full_line)
// //         sem_wait(line_s)

// //         // do operations

// //         sem_post(line_s)
// //         sem_post(full_line)

// //     }


// //     else{

// //         while( sem_get_value(full_line) != 0)
// //             wait(full_line)
        
// //         // do operations

// //         while( sem_get_value(full_line) != MAX_SAME_FILE_ACCESS)
// //             post(full_line)
        
// //         post(main_door)

// //     }

// // }




