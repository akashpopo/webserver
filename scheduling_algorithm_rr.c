#include <stdio.h>

#include "request_control_block.h"
#include "scheduler.h"
#include "scheduler_queue.h"

/* constants */
#define QUANTUM 8192

/* function prototypes */
static void submit(struct rcb* r);
static struct rcb* get_next(void);

/* initialize the queue */
static struct scheduler_queue ready;

/* initialize the RR scheduler struct */
struct scheduler_info rr_scheduler = {
    "RR",
    &submit,
    &get_next
};

/* inserts an RCB to the queue */
static void submit(struct rcb* r) {
  r->bytes_max_allowed = QUANTUM;                /* set quantum */
  scheduler_enqueue(&ready, r);        /* enqueue */
}

/* removes the current RCB and gets the next RCB */
static struct rcb* get_next(void) {
  return scheduler_dequeue(&ready);   /* dequeue and return */
}
