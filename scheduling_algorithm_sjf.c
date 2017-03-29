#include <stdio.h>

#include "request_control_block.h"
#include "scheduler.h"

/* function prototypes */
static void submit(struct rcb* req_control_block);
static struct rcb* get_next(void);

/* initialize the SJF scheduler struct */
struct scheduler_info sjf_scheduler = {
    "SJF",
    &submit,
    &get_next
};

/* keep track of the RCB's head */
static struct rcb *rcb_head;


/* inserts an RCB to the queue */
static void submit(struct rcb* req_control_block) {
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
static struct rcb* get_next(void) {
    struct rcb* req_control_block = rcb_head;        /* remove first item from the queue */
    if(req_control_block) {                      /* if queue is not empty */
        rcb_head = rcb_head->next_rcb;   /* unlink head */
        req_control_block->next_rcb = NULL;
    }
    return req_control_block;
}
