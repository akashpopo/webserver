#include "request_control_block.h"

/* Constants */
#define NUM_SUPPORTED_ALGORITHMS 3
#define MULTILEVEL_QUEUE_SIZE 3
#define HIGH_PRIORITY_QUANTUM 8192
#define LOW_PRIORITY_QUANTUM 65536

/* function prototypes */
extern void scheduler_init(char* selected_algorithm);
extern void submit_to_scheduler(struct rcb* request_control_block);
extern struct rcb* get_from_scheduler();
