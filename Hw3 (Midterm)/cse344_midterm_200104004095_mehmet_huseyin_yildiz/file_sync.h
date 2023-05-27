#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

#define MAX_PATH_LENGTH 1024

#define MAX_SAME_FILE_ACCESS 100



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

    if (lineNumber != 0) {
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

    
    if (lineNumber != 0) {
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


