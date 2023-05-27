// Queue implementation in C

# include "queue.h"

void enqueue(queue *q, void *value) {


    // Lock the queue mutex
    pthread_mutex_lock( q->mutex );

    if (q->size == MAX_QUEUE_SIZE) {
        printf("Queue is full\n");
        // Unlock the queue mutex
        pthread_mutex_unlock( q->mutex );
        return;
    }
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->size++;

    // Unlock the queue mutex
    pthread_mutex_unlock( q->mutex );

    sem_post(q->semaphore);

}

void *dequeue(queue *q) {


    sem_wait(q->semaphore);

    // Lock the queue mutex
    pthread_mutex_lock( q->mutex );

    if (q->size == 0) {
        printf("Queue is empty\n");

        // Unlock the queue mutex
        pthread_mutex_unlock( q->mutex );

        return NULL;
    }
    void *value = q->data[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    

    // Unlock the queue mutex
    pthread_mutex_unlock( q->mutex );

    return value;
}

void print_queue(queue *q) {
    printf("Queue (front to rear): ");
    for (int i = 0; i < q->size; i++) {
        int index = (q->front + i) % MAX_QUEUE_SIZE;
        printf("%s ", q->data[index]);
    }
    printf("\n");
}


// Queue Test 

// int main() {
//     queue q = { .front = 0, .rear = 0, .size = 0 };

//     char *str1 = "Hello";
//     char *str2 = "world";
//     char *str3 = "how";
//     char *str4 = "are";
//     char *str5 = "you";
//     char *str6 = "doing";
    
//     printf("before enqueue: %d\n",q.size);

//     enqueue(&q, str1);
//     enqueue(&q, str2);
//     enqueue(&q, str3);
//     enqueue(&q, str4);
//     enqueue(&q, str5);
//     enqueue(&q, str6);

//     printf("after enqueue: %d\n",q.size);

//     print_queue(&q);

//     printf("Dequeued item: %s\n", dequeue(&q));
//     printf("Dequeued item: %s\n", dequeue(&q));

//     printf("after dequeue: %d\n",q.size);

//     print_queue(&q);


//     return 0;
// }
