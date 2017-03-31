#ifndef SCHEDULER_QUEUE
#define SCHEDULER_QUEUE /* singleton pattern */

#include <pthread.h>

#include "request_control_block.h"

/* queue structure that keeps track of the RCB head and tail */
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
