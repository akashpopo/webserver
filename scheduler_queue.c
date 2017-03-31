#include <stdio.h>

#include "request_control_block.h"
#include "scheduler_queue.h"

static pthread_mutex_t mutex_initialize = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition_init = PTHREAD_COND_INITIALIZER;

/*
 * Initialize the work queue
 */
extern void queue_init(struct scheduler_queue* queue) {
    queue->lock = mutex_initialize;
    queue->not_empty = condition_init;
}

/*
 * Enqueues an RCB
 */
extern void scheduler_enqueue(struct scheduler_queue* queue, struct rcb* req_control_block) {
    req_control_block->next_rcb = NULL;

    //we are entering a critical section. Lock the state:
    pthread_mutex_lock(&queue->lock);
    if(!queue->head) {
        //queue is empty. Set the head:
        queue->head = req_control_block;
    } else {
        //queue is not empty. append the RCB to the tail:
        queue->tail->next_rcb = req_control_block;
    }

    //set the tail:
    queue->tail = req_control_block;

    //emit a signal that the queue is no longer empty:
    pthread_cond_signal(&queue->not_empty);

    //the critical section has ended. Unlock the state:
    pthread_mutex_unlock(&queue->lock);
}

/*
 * Removes and returns the first RCB in the queue
 */
extern struct rcb* scheduler_dequeue(struct scheduler_queue* queue, int wait) {
    struct rcb* req_control_block;

    //we are entering a critical section. Lock the state:
    pthread_mutex_lock(&queue->lock);
    if (wait && !queue->head) {
        //the queue is empty. Block it until it's not empty:
        pthread_cond_wait(&queue->not_empty, &queue->lock);
    }

    req_control_block = queue->head;
    if(queue->head) {
        //queue is not empty. Remove the first element:
        queue->head = queue->head->next_rcb;
        req_control_block->next_rcb = NULL;
    }
    //the critical section has ended. Unlock the state:
    pthread_mutex_unlock(&queue->lock);

    return req_control_block;
}
