#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>

#define TRUE  1
#define FALSE	0

/* a link in the queue, holds the info and point to the next Node*/
typedef struct {
    int info;
} DATA;

typedef struct Node_t {
    DATA data;
    struct Node_t *prev;
} NODE;

/* the HEAD of the Queue, hold the amount of node's that are in the queue*/
typedef struct Queue {
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;

Queue *create_queue(int limit);
void destruct_queue(Queue *queue);
int enqueue(Queue *pQueue, NODE *item);
NODE *dequeue(Queue *pQueue);
int is_empty(Queue* pQueue);

#endif
