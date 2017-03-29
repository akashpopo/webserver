#include <stdio.h>

#include "request_control_block.h"
#include "queue.h"

/* enqueues an RCB */
extern void queue_enqueue(struct queue* q, struct rcb* r) {
    r->next_rcb = NULL;
    if(!q->head) {              /* if empty */
        q->head = r;            /* set head */
    } else {
        q->tail->next_rcb = r;  /* add to tail */
    }
    q->tail = r;                /* set tail */
}

/* removes and returns the first RCB in the queue */
extern struct rcb* queue_dequeue(struct queue* q) {
    struct rcb *r = q->head;
    if(q->head) {                       /* if not empty */
        q->head = q->head->next_rcb;    /* remove first elemenet */
        r->next_rcb = NULL;
    }
    return r;
}
