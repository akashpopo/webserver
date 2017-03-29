#include <stdio.h>

#include "request_control_block.h"
#include "scheduler_queue.h"

/* enqueues an RCB */
extern void scheduler_enqueue(struct scheduler_queue* queue, struct rcb* req_control_block) {
    req_control_block->next_rcb = NULL;
    if(!queue->head) {              /* if empty */
        queue->head = req_control_block;            /* set head */
    } else {
        queue->tail->next_rcb = req_control_block;  /* add to tail */
    }
    queue->tail = req_control_block;                /* set tail */
}

/* removes and returns the first RCB in the queue */
extern struct rcb* scheduler_dequeue(struct scheduler_queue* queue) {
    struct rcb *req_control_block = queue->head;
    if(queue->head) {                       /* if not empty */
        queue->head = queue->head->next_rcb;    /* remove first elemenet */
        req_control_block->next_rcb = NULL;
    }
    return req_control_block;
}
