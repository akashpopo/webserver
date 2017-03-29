/*
 * File: rr.c
 * Author: Alex Brodsky
 * Purpose:  This fike implements the round robin scheduler
 */

#include <stdio.h>

#include "rcb.h"
#include "scheduler.h"
#include "queue.h"

#define QUANTUM 8192     /* Maximum send at one time */

static void submit(struct rcb* r);
static struct rcb* get_next(void);
static struct queue ready;

struct scheduler_info rr_scheduler = {
    "RR",
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
  r->max = QUANTUM;                /* set quantum */
  queue_enqueue(&ready, r);        /* enqueue */
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
static struct rcb* get_next(void) {
  return queue_dequeue(&ready);   /* dequeue and return */
}
