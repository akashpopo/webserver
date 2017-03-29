#include <stdio.h>

#include "rcb.h"
#include "scheduler.h"

/* function prototypes */
static void submit(struct rcb* r);
static struct rcb* get_next(void);

/* initialize the SJF scheduler struct */
struct scheduler_info sjf_scheduler = {
    "SJF",
    &submit,
    &get_next
};

/* keep track of the RCB's head */
static struct rcb *head;


/* inserts an RCB to the queue */
static void submit(struct rcb* r) {
    struct rcb *tmp;

    /* insert rcb into priority queue, based on # of bytes left to send. */
    if(!head || (r->bytes_remaining < head->bytes_remaining)) {  /* before head */
        r->next_rcb = head;
        head = r;
    } else {                                   /* after head */
        /* walk the list until the next rcb contains a lower priority request */
        for(tmp = head; tmp->next_rcb && (tmp->next_rcb->bytes_remaining <= r->bytes_remaining); tmp = tmp->next_rcb);
        /* insert RCB at this point */
        r->next_rcb = tmp->next_rcb;
        tmp->next_rcb = r;
    }
}

/* removes the current RCB and gets the next RCB */
static struct rcb* get_next(void) {
    struct rcb* r = head;        /* remove first item from the queue */
    if(r) {                      /* if queue is not empty */
        head = head->next_rcb;       /* unlink head */
        r->next_rcb = NULL;
    }
    return r;
}
