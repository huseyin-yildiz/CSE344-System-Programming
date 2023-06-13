// Queue Implementation

# ifndef QUEUE_H
# define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct {
    int max_queue_size;
    void **data;
    int front;
    int rear;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;

    short wait; 

}queue;

void init_queue(queue *q, int max_queue_size);
void destruct_queue(queue *q);
void enqueue(queue *q, void *value);
void *dequeue(queue *q);
void print_queue(queue *q);
void unblock_queue(queue* q);

# endif 
