#include <stdio.h>

#include "rcb.h"
#include "queue.h"


/* This function takes a pointer to a queue and a pointer to an RCB
 *    and enqueues the RCB onto the queue.
 * Parameters: q : pointer to a queue
               r : pointer to a RCB
 * Returns: None
 */
extern void queue_enqueue(struct queue* q, struct rcb* r) {
    r->next_rcb = NULL;
    if(!q->head) {         /* if empty */
        q->head = r;       /* set head */
    } else {
        q->tail->next_rcb = r; /* add to tail */
    }
    q->tail = r;           /* set tail */
}

/* This function takes a pointer to a queue and, and removes and returns
 *    the first RCB in the queue.  It returns NULL if the queue is empty.
 * Parameters: q: pointer to queue
 * Returns: Pointer to RCB removed from the queue.
 */
extern struct rcb* queue_dequeue(struct queue* q) {
    struct rcb *r = q->head;
    if(q->head) {                 /* if not empty */
        q->head = q->head->next_rcb;  /* remove first elemenet */
        r->next_rcb = NULL;
    }
    return r;
}
