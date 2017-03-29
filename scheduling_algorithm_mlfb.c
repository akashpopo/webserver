#include <stdio.h>

#include "request_control_block.h"
#include "scheduler.h"
#include "scheduler_queue.h"

/* constants */
#define HIGH_PRIORITY_QUANTUM 8192
#define LOW_PRIORITY_QUANTUM 65536

/* function prototypes */
static void submit(struct rcb* r);
static struct rcb* get_next(void);

/* initalize an array of 3 queues */
static struct scheduler_queue ready[3];

/* initialize the MLFB scheduler struct */
struct scheduler_info mlfb_scheduler = {
    "MLFB",
    &submit,
    &get_next
};

/* inserts an RCB to the queue */
static void submit(struct rcb* r) {
    if(r->bytes_max_allowed == 0) {                     /* select queue based on last send */
        r->bytes_max_allowed = HIGH_PRIORITY_QUANTUM;   /* set quantum */
        scheduler_enqueue(&ready[0], r);                    /* add to queue */
    } else if(r->bytes_max_allowed == HIGH_PRIORITY_QUANTUM) {
        r->bytes_max_allowed = LOW_PRIORITY_QUANTUM;
        scheduler_enqueue(&ready[1], r);
    } else {
        scheduler_enqueue(&ready[2], r);
    }
}

/* removes the current RCB and gets the next RCB */
static struct rcb* get_next(void) {
    struct rcb* r;
    for (int i = 0; i < 3; i++) {       /* loop throught all 3 queues */
        r = scheduler_dequeue(&ready[i]);   /* attempt to dequeue */
        if (r) {                        /* if success, then use RCB */
            break;
        }
    }
    return r;
}
