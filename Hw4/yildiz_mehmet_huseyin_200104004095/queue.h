// Queue Implementation

# ifndef QUEUE_H
# define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_QUEUE_SIZE 100

typedef struct {
    void *data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
    pthread_mutex_t* mutex;
    sem_t* semaphore;            

}queue;


void enqueue(queue *q, void *value);
void *dequeue(queue *q);
void print_queue(queue *q);

# endif 
