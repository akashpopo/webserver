/*
 * File: sjf.c
 * Author: Alex Brodsky
 * Purpose:   This is an implementation of the Shortest Job First Scheduler.
 */

#include <stdio.h>

#include "rcb.h"
#include "scheduler.h"

static void submit(struct rcb* r);
static struct rcb* get_next(void);

struct scheduler_info sjf_scheduler = {
    "SJF",
    &submit,
    &get_next
};

static struct rcb *head; /* head of ready queue */

/* This function checks if there are any web clients waiting to connect.
 *    If one or more clients are waiting to connect, this function returns.
 *    Otherwise, this function puts the program to sleep (blocks) until
 *    a client connects.
 * Parameters: None
 * Returns: None
 */
static void submit(struct rcb* r) {
    struct rcb *tmp;

    /* insert rcb into priority queue, based on # of bytes left to send. */
    if(!head || (r->left < head->left)) {  /* before head */
        r->next = head;
        head = r;
    } else {                                   /* after head */
        /* walk the list until the next rcb contains a lower priority request */
        for(tmp = head; tmp->next && (tmp->next->left <= r->left); tmp = tmp->next);

        /* insert RCB at this point */
        r->next = tmp->next;
        tmp->next = r;
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
static struct rcb* get_next(void) {
    struct rcb* r = head;        /* remove first item from the queue */
    if(r) {                      /* if queue is not empty */
        head = head->next;       /* unlink head */
        r->next = NULL;
    }
    return r;
}
