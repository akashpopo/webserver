#include <stdio.h>

#include "request_control_block.h"
#include "scheduler.h"
#include "scheduler_queue.h"

/* constants */
#define QUANTUM 8192

/* function prototypes */
static void submit(struct rcb* req_control_block);
static struct rcb* get_next(void);

/* initialize the queue */
static struct scheduler_queue round_robin;

/* initialize the RR scheduler struct */
struct scheduler_info round_robin_scheduler = {
    "RR",
    &submit,
    &get_next
};

/* inserts an RCB to the queue */
static void submit(struct rcb* req_control_block) {
  req_control_block->bytes_max_allowed = QUANTUM;                /* set quantum */
  scheduler_enqueue(&round_robin, req_control_block);        /* enqueue */
}

/* removes the current RCB and gets the next RCB */
static struct rcb* get_next(void) {
  return scheduler_dequeue(&round_robin);   /* dequeue and return */
}
