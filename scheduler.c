#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "request_control_block.h"
#include "scheduler.h"

/* declare supported schedulers */
extern struct scheduler_info shortest_job_first_scheduler;
extern struct scheduler_info round_robin_scheduler;
extern struct scheduler_info multilevel_scheduler;

/* setup an array of all supported schedulers */
static struct scheduler_info* supported_scheduler_algorithms[] = {
    &shortest_job_first_scheduler,
    &round_robin_scheduler,
    &multilevel_scheduler
};

/* keep track of the selected scheduler */
static struct scheduler_info* selected_scheduler;

/* init and set the scheduler */
extern void scheduler_init(char* selected_algorithm) {
    // find the scheduler set by user input
    for (int i = 0; i < NUM_SUPPORTED_ALGORITHMS; i++) {
        char* curr_algorithm = supported_scheduler_algorithms[i]->algorithm;
        if (!strcmp(selected_algorithm, curr_algorithm)) {
            // we found the scheduler algorithm
            selected_scheduler = supported_scheduler_algorithms[i];
            return;
        }
    }
    // the algorithm did not match the supported scheduling algorithms
    printf("This webserver does not support the %s scheduling algorithm.\n", selected_algorithm);
    printf("Only SJF, RR, and MLFB are supported. Terminating...\n");
    abort();
}

/* Submits a RCB to the scheduler */
extern void submit_to_scheduler(struct rcb* request_control_block) {
    selected_scheduler->submit_rcb(request_control_block);
}

/* Removes the RCB and gets the next RCB */
extern struct rcb* get_from_scheduler() {
    return selected_scheduler->get_next_rcb();
}
