#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "request_control_block.h"
#include "scheduler.h"
#include "scheduler_queue.h"

/* Function Prototypes: */
static void sjf_submit(struct rcb* req_control_block); // Shortest Job First
static struct rcb* sjf_get_next(void);
static void rr_init();                                 // Round Robin
static void rr_submit(struct rcb* req_control_block);
static struct rcb* rr_get_next(void);
static void mflb_init();                               // Multilevel Feedback
static void mlfb_submit(struct rcb* req_control_block);
static struct rcb* mlfb_get_next(void);

/* global variables */
char* scheduler_algorithm_selected;          // declare the string that keeps track of the chosen algorithm
static struct scheduler_queue round_robin;   // declare the queue used by RR
static struct rcb* rcb_head;                 // declare a pointer to the RCB head used by SJF
static struct scheduler_queue multilevel[MULTILEVEL_QUEUE_SIZE]; // declare the multilevel queue used by MLFB
static pthread_mutex_t sjf_lock = PTHREAD_COND_INITIALIZER;

/*
 * initialize and set the scheduler
 *
 * Source:
 * Understanding "extern" keyword in C. (2017, March 27).
 * Retrieved April 02, 2017, from http://www.geeksforgeeks.org/understanding-extern-keyword-in-c/
 */
extern void scheduler_init(char* selected_algorithm) {
    if (!strcmp(selected_algorithm, "SJF")) {
        scheduler_algorithm_selected = "SJF";
        return;
    }
    else if (!strcmp(selected_algorithm, "RR")) {
        scheduler_algorithm_selected = "RR";
        rr_init();
        return;
    }
    else if (!strcmp(selected_algorithm, "MLFB")) {
        scheduler_algorithm_selected = "MLFB";
        mflb_init();
        return;
    }
    else {
        printf("This webserver does not support the %s scheduling algorithm.\n", selected_algorithm);
        printf("Only SJF, RR, and MLFB are supported. Terminating...\n");
        abort();
    }
}

/*
 * Submits a RCB to the scheduler
 *
 * Source:
 * Understanding "extern" keyword in C. (2017, March 27).
 * Retrieved April 02, 2017, from http://www.geeksforgeeks.org/understanding-extern-keyword-in-c/
 */
extern void submit_to_scheduler(struct rcb* request_control_block) {
    if (!strcmp(scheduler_algorithm_selected, "SJF")) {
        sjf_submit(request_control_block);
        return;
    }
    else if (!strcmp(scheduler_algorithm_selected, "RR")) {
        rr_submit(request_control_block);
        return;
    }
    else if (!strcmp(scheduler_algorithm_selected, "MLFB")) {
        mlfb_submit(request_control_block);
        return;
    }
    else {
        printf("Scheduling algorithm: %s\n", scheduler_algorithm_selected);
        printf("Error in submit_to_scheduler. Terminating...\n");
        abort();
    }
}

/*
 * Removes the RCB and gets the next RCB
 *
 * Source:
 * Understanding "extern" keyword in C. (2017, March 27).
 * Retrieved April 02, 2017, from http://www.geeksforgeeks.org/understanding-extern-keyword-in-c/
 */
extern struct rcb* get_from_scheduler() {
    if (!strcmp(scheduler_algorithm_selected, "SJF")) {
        return sjf_get_next();
    }
    else if (!strcmp(scheduler_algorithm_selected, "RR")) {
        return rr_get_next();
    }
    else if (!strcmp(scheduler_algorithm_selected, "MLFB")) {
        return mlfb_get_next();
    }
    else {
        printf("Scheduling algorithm: %s\n", scheduler_algorithm_selected);
        printf("Error in get_from_scheduler. Terminating...\n");
        abort();
    }
}



/* SHORTEST JOB FIRST: */
/********************************************************************************/

/*
 * Inserts an RCB to the queue using SJF
 */
static void sjf_submit(struct rcb* req_control_block) {
    struct rcb* temp_rcb;

    // we are entering a critical section. Lock the state:
    pthread_mutex_lock(&sjf_lock);

    if (!rcb_head || (req_control_block->bytes_remaining < rcb_head->bytes_remaining)) {
        // before the head. Set it to head:
        req_control_block->next_rcb = rcb_head;
        rcb_head = req_control_block;
    }
    else {
        // after the head. Find the spot where the RCB should be inserted by looping through the list
        for(temp_rcb = rcb_head; temp_rcb->next_rcb && (temp_rcb->next_rcb->bytes_remaining <= req_control_block->bytes_remaining); temp_rcb = temp_rcb->next_rcb);

        // we have found the spot. Insert it here:
        req_control_block->next_rcb = temp_rcb->next_rcb;
        temp_rcb->next_rcb = req_control_block;
    }

    // we are leaving the critical section. Unlock the state:
    pthread_mutex_unlock(&sjf_lock);
}

/*
 * Removes the current RCB and gets the next RCB using SJF
 */
static struct rcb* sjf_get_next(void) {
    struct rcb* req_control_block;

    // we are entering a critical section. Lock the state:
    pthread_mutex_lock(&sjf_lock);

    // remove the current item:
    req_control_block = rcb_head;

    if(req_control_block) {
        // queue is not empty. Remove the head link:
        rcb_head = rcb_head->next_rcb;
        req_control_block->next_rcb = NULL;
    }

    // we are leaving the critical section. Unlock the state:
    pthread_mutex_unlock(&sjf_lock);

    return req_control_block;
}



/* ROUND ROBIN: */
/********************************************************************************/

/*
 * Inserts an RCB to the queue using RR
 */
 static void rr_init() {
     queue_init(&round_robin);
 }

/*
 * Inserts an RCB to the queue using RR
 */
static void rr_submit(struct rcb* req_control_block) {
    req_control_block->bytes_max_allowed = HIGH_PRIORITY_QUANTUM;
    scheduler_enqueue(&round_robin, req_control_block);
}

/*
 * Removes the current RCB and gets the next RCB using RR
 */
static struct rcb* rr_get_next(void) {
    return scheduler_dequeue(&round_robin, 0);
}



/* MULTILEVEL FEEDBACK: */
/********************************************************************************/

/*
 * Initialize the multilevel ready queues:
 */
 static void mflb_init() {
     // init all 3 multilevel queues:
     for (int i = 0; i < MULTILEVEL_QUEUE_SIZE; i++) {
         queue_init(&multilevel[i]);
     }
 }

/*
 * Inserts an RCB to the queue using MLFB
 */
static void mlfb_submit(struct rcb* req_control_block) {

    // select which queue to use based on the last transmission:
    if(req_control_block->bytes_max_allowed == 0) {
        // set the quantum and add it to the first queue:
        req_control_block->bytes_max_allowed = HIGH_PRIORITY_QUANTUM;
        scheduler_enqueue(&multilevel[0], req_control_block);
    }
    else if(req_control_block->bytes_max_allowed == HIGH_PRIORITY_QUANTUM) {
        // set the quantum and add it to the second queue:
        req_control_block->bytes_max_allowed = LOW_PRIORITY_QUANTUM;
        scheduler_enqueue(&multilevel[1], req_control_block);
    }
    else {
        // add it to the third queue:
        scheduler_enqueue(&multilevel[2], req_control_block);
    }
}

/*
 * Removes the current RCB and gets the next RCB using MLFB
 */
static struct rcb* mlfb_get_next(void) {
    struct rcb* req_control_block;

    // attempt to dequeue each queue:
    for (int i = 0; i < MULTILEVEL_QUEUE_SIZE; i++) {
        // attempt to dequeue:
        req_control_block = scheduler_dequeue(&multilevel[i], 0);
        if (req_control_block) {
            // successfully dequeued. RCB found:
            break;
        }
    }

    return req_control_block;
}
