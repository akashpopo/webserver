#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rcb.h"
#include "scheduler.h"

/* declare supported schedulers */
extern struct scheduler_info sjf_scheduler;
extern struct scheduler_info rr_scheduler;
extern struct scheduler_info mlfb_scheduler;

/* setup an array of all supported schedulers */
static struct scheduler_info* supported_schedulers[] = {
    &sjf_scheduler,
    &rr_scheduler,
    &mlfb_scheduler
};

/* keep track of the selected scheduler */
static struct scheduler_info* selected_scheduler;

/* init and set the scheduler */
extern void scheduler_init(char* selected_algorithm) {
    for (int i = 0; i < NUM_SUPPORTED_ALGORITHMS; i++) {
        if (!strcmp(selected_algorithm, supported_schedulers[i]->algorithm)) {
            selected_scheduler = supported_schedulers[i];
            return;
        }
    }
    printf("This webserver does not support the %s scheduling algorithm.\n", selected_algorithm);
    printf("Only SJF, RR, and MLFB are supported. Terminating...\n");
    abort();
}

/* Submits a RCB to the scheduler */
extern void scheduler_submit(struct rcb* request_control_block) {
    selected_scheduler->submit(request_control_block);
}

/* Removes the RCB and gets the next RCB */
extern struct rcb* scheduler_get_next() {
    return selected_scheduler->get_next();
}
