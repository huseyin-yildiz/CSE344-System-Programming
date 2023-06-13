#include "file_list.h"

void traverse_directory(const char* directory_path, FileInfo** file_list, int* count) {
    DIR* dir = opendir(directory_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignore current and parent directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[MAX_PATH_LENGTH];
        sprintf(full_path, "%s/%s", directory_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // Recursive call for subdirectories
            traverse_directory(full_path, file_list, count);
        } else if (entry->d_type == DT_REG) {
            // Regular file, calculate hash
            FileInfo file_info;
            const char* dr = strchr(full_path, '/') + 1;
            strcpy(file_info.path, dr);
            calculate_file_hash(full_path, file_info.hash);

            *file_list = realloc(*file_list, (*count + 1) * sizeof(FileInfo));
            (*file_list)[*count] = file_info;
            (*count)++;
        }
    }

    closedir(dir);
}

void calculate_file_hash(const char* file_path, unsigned char* hash) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        perror("fopen");
        return;
    }

    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);

    unsigned char buffer[SHA256_DIGEST_LENGTH];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, SHA256_DIGEST_LENGTH, file)) != 0) {
        SHA256_Update(&sha256_ctx, buffer, bytes_read);
    }

    SHA256_Final(hash, &sha256_ctx);

    fclose(file);
}




void send_file_list(int socket_fd, FileInfo* file_list, int count) {
    ssize_t result = send(socket_fd, &count, sizeof(count), 0);
    if (result == -1) {
        perror("socket send_file_list");
        // Handle error
        return;
    }

    result = send(socket_fd, file_list, count * sizeof(FileInfo), 0);
    if (result == -1) {
        perror("socket send_file_list");
    }
}




FileInfo* receive_file_list(int socket_fd, int* count) {
    ssize_t result = recv(socket_fd, count, sizeof(*count), 0);
    if (result == -1) {
        // Handle error
        return NULL;
    }

    FileInfo* file_list = (FileInfo*)malloc(*count * sizeof(FileInfo));
    if (file_list == NULL) {
        // Handle memory allocation error
        return NULL;
    }

    size_t bytesReceived = 0;
    size_t bytes_to_recv = (*count) * sizeof(FileInfo);

    while (bytesReceived < bytes_to_recv ) {
        result = recv(socket_fd, file_list + bytesReceived  , bytes_to_recv , 0);
        
        if (result == -1) {
            // Handle error
            free(file_list);
            return NULL;
        }
        
        bytesReceived += result;
        bytes_to_recv -= result;
    }
    
    return file_list;
}


// Create directory recursively
void make_dir(char* file_path) {
    char* dir = strdup(file_path);
    char* end = strrchr(dir, '/');
    if (end != NULL) {
        *end = '\0'; // Truncate the path at the last '/'
        make_dir(dir); // Recursive call to create parent directories
        *end = '/';
    }
    mkdir(dir, 0700);
    
    free(dir);
}


// int main() {
//     const char* directory_path = "test";
//     FileInfo* file_list = NULL;
//     int count = 0;

//     traverse_directory(directory_path, &file_list, &count);

//     // Print the file paths and hashes
//     for (int i = 0; i < count; i++) {
//         // printf("File Path: %s\n", file_list[i].path);
//         // printf("Hash: ");
//         for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
//         //     printf("%02x", file_list[i].hash[j]);
            
//         // }
//         printf("%x.: %x path:%x hash:%x",i,&file_list[i], &(file_list[i].path), &(file_list[i].hash)  );
//         printf("\n\n");
        
//     }

//     // Cleanup
//     free(file_list);

//     return 0;
// }
