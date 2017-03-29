/*
 * File: network.c
 * Author: Alex Brodsky
 * Purpose: This file contains the scheduler layer, which acts as common
 *          layer between sws.c and the three schedulers.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rcb.h"
#include "scheduler.h"

extern  vss sjf_scheduler;
extern  vss rr_scheduler;
extern  vss mlfb_scheduler;

static  vss * schedulers[] = { &sjf_scheduler,   /* SJF Scheduler vss */
                                &rr_scheduler,   /* RR Scheduler vss */
                                &mlfb_scheduler, /* MLFB Scheduler vss */
                                NULL };
static  vss * scheduler;                         /* selected scheduler */



/* This function initializes the scheduler module and sets the scheduler.
 *   This function will abort if the scheduler is not known.
 * Parameters:
 *             name : the name of the scheduler
 * Returns: None
 */
extern void scheduler_init( char * name ) {
  int i;

  for( i = 0; name && schedulers[i]; i++ ) {      /* loop through vss records */
    if( !strcmp( name, schedulers[i]->name ) ) {  /* Check if scheduler found */
      scheduler = schedulers[i];                  /* Set scheduler */
      return;
    }
  }
  printf( "Unknown scheduler: %s\n", name );      /* Scheduler not found */
  abort();
}


/* This function submits an RCB to the scheduler to be inserted into a
 *    ready queue.
 * Parameters: req : pointer to RCB
 * Returns: None
 */
extern void scheduler_submit( rcb * req ) {
  assert( scheduler );
  scheduler->submit( req );           /* redirect call to selected scheduler */
}


/* This function removes and returns the next RCB in the ready queue.
 *    If the queue is empty, NULL is returned.
 * Parameters: None
 * Returns: Pointer to an RCB, NULL if queue is empty.
 */
extern rcb * scheduler_get_next() {
  assert( scheduler );
  return scheduler->get_next();     /* redirect call to selected scheduler */
}
