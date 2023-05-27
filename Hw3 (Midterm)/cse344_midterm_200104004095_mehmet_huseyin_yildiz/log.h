#include <stdio.h>
#include <stdlib.h>
#include <time.h>

FILE* openLogFile(const char* pid) {
    // Generate timestamp
    time_t rawTime;
    struct tm* timeInfo;
    char timestamp[20];

    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", timeInfo);

    // Create log file name with timestamp
    char logFileName[50];
    sprintf(logFileName, "./logs/%s_%s.log",pid, timestamp);

    // Open the log file for writing
    FILE* logFile = fopen(logFileName, "w");
    if (logFile == NULL) {
        perror("fopen");
        return NULL;
    }

    // Return the log file pointer
    return logFile;
}


void logMessage(FILE* file, const char* message) {
    // Get the current time
    time_t rawTime;
    struct tm* timeInfo;
    char timestamp[20];

    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeInfo);

    // Print the timestamp and message to the file
    fprintf(file, "[%s] %s\n", timestamp, message);

    // Flush the file to ensure the message is written immediately
    fflush(file);
}