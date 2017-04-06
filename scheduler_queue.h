/*
 * Singleton pattern
 *
 * Source:
 * Singleton Pattern in C. (n.d.). Retrieved March 28, 2017,
 * from http://stackoverflow.com/questions/5171661/singleton-pattern-in-c
 */
#ifndef SCHEDULER_QUEUE
#define SCHEDULER_QUEUE

#include <pthread.h>

#include "request_control_block.h"

/*
 * Queue structure that keeps track of the RCB head and tail
 *
 * Source:
 * Mutex Lock Code Examples (Multithreaded Programming Guide). (n.d.).
 * Retrieved March 26, 2017, from https://docs.oracle.com/cd/E19683-01/806-6867/sync-12/index.html
 */
struct scheduler_queue {
    struct rcb* head;
    struct rcb* tail;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
};

/* function prototypes */
extern void queue_init(struct scheduler_queue* queue);
extern void scheduler_enqueue(struct scheduler_queue* queue, struct rcb* req_control_block);
extern struct rcb* scheduler_dequeue(struct scheduler_queue* queue, int wait);

#endif
