/*
 * File: scheduler.h
 * Author: Alex Brodsky
 * Purpose: Defines the scheduler interface.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "rcb.h"

typedef void (*submit_func)( rcb * );      /* function prototype for submit */
typedef rcb * (*get_next_func)( void );    /* function prototype for get_next */

typedef struct _vss vss;
struct _vss {
  char *        name;                      /* name of scheduler */
  submit_func   submit;                    /* func ptr to submit func */
  get_next_func get_next;                  /* func ptr tp get_next func */
};

/*
 * This module has three functions:
 *   scheduler_init() : sets and inititalizes the required scheduler
 *   scheduler_submkt() : submit (or resubmit) a request to the scheduler
 *   schdeduler_get_next() : gets the next request to service
 *
 * The scheduler_init() function should be called once, at the start of the
 * program.  This function will set which scheduler to use.
 *
 * The scheduler_submit() function should be called when a new job is to be
 * be added to the ready queue.  This function takes a pointer to an RCB and
 * adds it to the appropriate queue.
 *
 * The scheduler_get_next() function removes the next RCB from the ready queue
 * and returns a pointer to it.  If the queue is empty, NULL is returned.
 */


/* This function initializes the scheduler module and sets the scheduler.
 *   This function will abort if the scheduler is not known.
 * Parameters:
 *             name : the name of the scheduler
 * Returns: None
 */
extern void scheduler_init( char * name );


/* This function submits an RCB to the scheduler to be inserted into a
 *    ready queue.
 * Parameters: req : pointer to RCB
 * Returns: None
 */
extern void scheduler_submit( rcb * req );


/* This function removes and returns the next RCB in the ready queue.
 *    If the queue is empty, NULL is returned.
 * Parameters: None
 * Returns: Pointer to an RCB, NULL if queue is empty.
 */
extern rcb * scheduler_get_next();

#endif
