/*
 * File: queue.c
 * Author: Alex Brodsky
 * Purpose:  This implements a general RCB queue with a linked list
 */

#include <stdio.h>
#include <assert.h>

#include "rcb.h"
#include "queue.h"


/* This function takes a pointer to a queue and a pointer to an RCB
 *    and enqueues the RCB onto the queue.
 * Parameters: q : pointer to a queue
               r : pointer to a RCB
 * Returns: None
 */
extern void queue_enqueue( queue * q, rcb * r ) {
  assert( q );
  assert( r );

  r->next = NULL;
  if( !q->head ) {         /* if emptty */
    q->head = r;           /* set head */
  } else {                 /* else */
    q->tail->next = r;     /* add to tail */
  }
  q->tail = r;             /* set tail */
}

/* This function takes a pointer to a queue and, and removes and returns
 *    the first RCB in the queue.  It returns NULL if the queue is empty.
 * Parameters: q: pointer to queue
 * Returns: Pointer to RCB removed from the queue.
 */
extern rcb * queue_dequeue( queue * q ) {
  rcb *r = q->head;

  if( q->head ) {             /* if not empty */
    q->head = q->head->next;  /* remove first elemenet */
    r->next = NULL;
  }
  return r;
}
