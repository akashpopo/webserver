#ifndef QUEUE
#define QUEUE

#include "rcb.h"

/* queue structure that keeps track of the RCB head and tail */
struct queue {
    struct rcb* head;
    struct rcb* tail;
};

/* function prototypes */
extern void queue_enqueue(struct queue* q, struct rcb* r);
extern struct rcb* queue_dequeue(struct queue* q);

#endif
