/*
 * File: rr.c
 * Author: Alex Brodsky
 * Purpose: This file implements the MLFB scheduler.
 */

#include <stdio.h>

#include "rcb.h"
#include "scheduler.h"
#include "queue.h"

#define HIGH_PRIORITY_QUANTUM 8192
#define LOW_PRIORITY_QUANTUM 65536

static void submit(struct rcb* r);
static struct rcb* get_next(void);
static struct queue ready[3];

struct scheduler_info mlfb_scheduler = {
    "MLFB",
    &submit,
    &get_next
};


/* This function checks if there are any web clients waiting to connect.
 *    If one or more clients are waiting to connect, this function returns.
 *    Otherwise, this function puts the program to sleep (blocks) until
 *    a client connects.
 * Parameters: None
 * Returns: None
 */
static void submit(struct rcb* r) {
    if(r->bytes_max_allowed == 0) {                     /* select queue based on last send */
        r->bytes_max_allowed = HIGH_PRIORITY_QUANTUM;   /* set quantum */
        queue_enqueue(&ready[0], r);      /* add to queue */
    } else if(r->bytes_max_allowed == HIGH_PRIORITY_QUANTUM) {
        r->bytes_max_allowed = LOW_PRIORITY_QUANTUM;
        queue_enqueue(&ready[1], r);
    } else {
        queue_enqueue(&ready[2], r);
    }
}


/* This function checks if there are any web clients waiting to connect.
 *    If one or more clients are waiting to connect, this function opens
 *    a connection to the next client waiting to connect, and returns an
 *    integer file descriptor for the connection.  If no clients are
 *    waiting, this function returns -1.
 * Parameters: None
 * Returns: A positive integer file decriptor to the next clients connection,
 *          or -1 if no client is waiting.
 */
static struct rcb* get_next( void ) {
    struct rcb* r;
    for (int i = 0; i < 3; i++) {       /* loop throught all 3 queues */
        r = queue_dequeue(&ready[i]);   /* attempt to dequeue */
        if (r) {                        /* if success, then use RCB */
            break;
        }
    }
    return r;
}
