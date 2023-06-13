// Queue implementation in C

# include "queue.h"

void init_queue(queue *q, int max_size){

    q->max_queue_size = max_size;
    q->data = malloc(max_size * sizeof(void*));
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->wait = 1;

    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void destruct_queue(queue *q){

    free(q->data);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
}

// Add an element to the queue 
void enqueue(queue *q, void *value) {

    // Lock the queue mutex
    pthread_mutex_lock( &(q->mutex) );

    // Wait until the queue is has an empty space
    while (q->size == q->max_queue_size) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    
    // Add the element to the queue
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % q->max_queue_size;
    q->size++;

    // Signal the not_empty
    pthread_cond_signal(&q->not_empty);

    // Unlock the queue mutex
    pthread_mutex_unlock( &q->mutex );
}

// Remove element and return it from the queue
void *dequeue(queue *q) {

    // Lock the queue mutex
    pthread_mutex_lock( &q->mutex );

    // If the wait flag is enabled wait until there is a at least one element 
    while (q->size == 0 && q->wait) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    // If the size is 0 and wait is disabled
    if(q->size == 0)
    {
        pthread_mutex_unlock( &q->mutex );
        return NULL;
    }

    void *value = q->data[q->front];
    q->front = (q->front + 1) % q->max_queue_size;
    q->size--;
    
    pthread_cond_signal(&q->not_full);

    // Unlock the queue mutex
    pthread_mutex_unlock( &q->mutex );
    
    return value;
}

// Unblock all waited dequeue func callers and return NULL value to them
void unblock_queue(queue* q) {
    pthread_mutex_lock(&q->mutex);
    q->wait = 0;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}


void print_queue(queue *q) {
    printf("Queue (front to rear): ");
    for (int i = 0; i < q->size; i++) {
        int index = (q->front + i) % q->max_queue_size;
        printf("%s ", (char*)q->data[index]);
    }
    printf("\n");
}


// Queue Test 

// int main() {
//     queue q;
//     init_queue(&q, 10);

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

//     printf("Dequeued item: %s\n", (char*)dequeue(&q));
//     printf("Dequeued item: %s\n", (char*)dequeue(&q));

//     printf("after dequeue: %d\n",q.size);

//     print_queue(&q);


//     return 0;
// }
