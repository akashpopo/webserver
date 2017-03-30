#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "request_control_block.h"
#include "scheduler.h"
#include "scheduler_queue.h"

/* Shortest Job First */
static void sjf_submit(struct rcb* req_control_block);
static struct rcb* sjf_get_next(void);

/* Round Robin */
static void rr_submit(struct rcb* req_control_block);
static struct rcb* rr_get_next(void);

/* Multilevel Feedback */
static void mlfb_submit(struct rcb* req_control_block);
static struct rcb* mlfb_get_next(void);


/* global variables */
char* scheduler_algorithm_selected;

/* init and set the scheduler */
extern void scheduler_init(char* selected_algorithm) {
    if (!strcmp(selected_algorithm, "SJF")) {
        scheduler_algorithm_selected = "SJF";
        return;
    } else if (!strcmp(selected_algorithm, "RR")) {
        scheduler_algorithm_selected = "RR";
        return;
    } else if (!strcmp(selected_algorithm, "MLFB")) {
        scheduler_algorithm_selected = "MLFB";
        return;
    } else {
        // the algorithm did not match the supported scheduling algorithms
        printf("This webserver does not support the %s scheduling algorithm.\n", selected_algorithm);
        printf("Only SJF, RR, and MLFB are supported. Terminating...\n");
        abort();
    }
}

/* Submits a RCB to the scheduler */
extern void submit_to_scheduler(struct rcb* request_control_block) {
    if (!strcmp(scheduler_algorithm_selected, "SJF")) {
        sjf_submit(request_control_block);
        return;
    } else if (!strcmp(scheduler_algorithm_selected, "RR")) {
        rr_submit(request_control_block);
        return;
    } else if (!strcmp(scheduler_algorithm_selected, "MLFB")) {
        mlfb_submit(request_control_block);
        return;
    } else {
        printf("Scheduling algorithm: %s\n", scheduler_algorithm_selected);
        printf("Error in submit_to_scheduler. Terminating...\n");
        abort();
    }
}

/* Removes the RCB and gets the next RCB */
extern struct rcb* get_from_scheduler() {
    if (!strcmp(scheduler_algorithm_selected, "SJF")) {
        return sjf_get_next();
    } else if (!strcmp(scheduler_algorithm_selected, "RR")) {
        return rr_get_next();
    } else if (!strcmp(scheduler_algorithm_selected, "MLFB")) {
        return mlfb_get_next();
    } else {
        printf("Scheduling algorithm: %s\n", scheduler_algorithm_selected);
        printf("Error in get_from_scheduler. Terminating...\n");
        abort();
    }
}


/********************************************************************************/
/* initialize the queue */
static struct scheduler_queue round_robin;

/* ROUND ROBIN */
/* inserts an RCB to the queue */
static void rr_submit(struct rcb* req_control_block) {
    req_control_block->bytes_max_allowed = QUANTUM;       /* set quantum */
    scheduler_enqueue(&round_robin, req_control_block);   /* enqueue */
}

/* removes the current RCB and gets the next RCB */
static struct rcb* rr_get_next(void) {
    return scheduler_dequeue(&round_robin);   /* dequeue and return */
}

/********************************************************************************/

/* SHORTEST JOB FIRST */
/* keep track of the RCB's head */
static struct rcb *rcb_head;

/* inserts an RCB to the queue */
static void sjf_submit(struct rcb* req_control_block) {
    struct rcb *temp_rcb;
    /* insert rcb into priority queue, based on # of bytes left to send. */
    if(!rcb_head || (req_control_block->bytes_remaining < rcb_head->bytes_remaining)) {  /* before head */
        req_control_block->next_rcb = rcb_head;
        rcb_head = req_control_block;
    } else {                                                     /* after head */
        /* walk the list until the next rcb contains a lower priority request */
        for(temp_rcb = rcb_head; temp_rcb->next_rcb && (temp_rcb->next_rcb->bytes_remaining <= req_control_block->bytes_remaining); temp_rcb = temp_rcb->next_rcb);
        /* insert RCB at this point */
        req_control_block->next_rcb = temp_rcb->next_rcb;
        temp_rcb->next_rcb = req_control_block;
    }
}

/* removes the current RCB and gets the next RCB */
static struct rcb* sjf_get_next(void) {
    struct rcb* req_control_block = rcb_head;    /* remove first item from the queue */
    if(req_control_block) {                      /* if queue is not empty */
        rcb_head = rcb_head->next_rcb;           /* unlink head */
        req_control_block->next_rcb = NULL;
    }
    return req_control_block;
}

/********************************************************************************/

/* MULTILEVEL FEEDBACK QUEUE */
/* initalize an array of 3 queues */
static struct scheduler_queue multilevel[3];

/* inserts an RCB to the queue */
static void mlfb_submit(struct rcb* req_control_block) {
    if(req_control_block->bytes_max_allowed == 0) {                     /* select queue based on last send */
        req_control_block->bytes_max_allowed = HIGH_PRIORITY_QUANTUM;   /* set quantum */
        scheduler_enqueue(&multilevel[0], req_control_block);                    /* add to queue */
    } else if(req_control_block->bytes_max_allowed == HIGH_PRIORITY_QUANTUM) {
        req_control_block->bytes_max_allowed = LOW_PRIORITY_QUANTUM;
        scheduler_enqueue(&multilevel[1], req_control_block);
    } else {
        scheduler_enqueue(&multilevel[2], req_control_block);
    }
}

/* removes the current RCB and gets the next RCB */
static struct rcb* mlfb_get_next(void) {
    struct rcb* req_control_block;
    for (int i = 0; i < 3; i++) {       /* loop throught all 3 queues */
        req_control_block = scheduler_dequeue(&multilevel[i]);   /* attempt to dequeue */
        if (req_control_block) {                        /* if success, then use RCB */
            break;
        }
    }
    return req_control_block;
}

/********************************************************************************/
