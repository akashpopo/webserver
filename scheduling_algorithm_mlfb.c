#include <stdio.h>

#include "request_control_block.h"
#include "scheduler.h"
#include "scheduler_queue.h"

/* constants */
#define HIGH_PRIORITY_QUANTUM 8192
#define LOW_PRIORITY_QUANTUM 65536

/* function prototypes */
static void submit_rcb(struct rcb* req_control_block);
static struct rcb* get_next_rcb(void);

/* initalize an array of 3 queues */
static struct scheduler_queue multilevel[3];

/* initialize the MLFB scheduler struct */
struct scheduler_info multilevel_scheduler = {
    "MLFB",
    &submit_rcb,
    &get_next_rcb
};

/* inserts an RCB to the queue */
static void submit_rcb(struct rcb* req_control_block) {
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
static struct rcb* get_next_rcb(void) {
    struct rcb* req_control_block;
    for (int i = 0; i < 3; i++) {       /* loop throught all 3 queues */
        req_control_block = scheduler_dequeue(&multilevel[i]);   /* attempt to dequeue */
        if (req_control_block) {                        /* if success, then use RCB */
            break;
        }
    }
    return req_control_block;
}
