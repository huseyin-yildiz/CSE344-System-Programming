// Queue Implementation

# ifndef QUEUE_H
# define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_QUEUE_SIZE 10

typedef struct {
    char *data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
}queue;


void enqueue(queue *q, char *value);
char *dequeue(queue *q);
void print_queue(queue *q);

# endif 
