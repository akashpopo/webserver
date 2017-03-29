#include "request_control_block.h"

/* Constants */
#define NUM_SUPPORTED_ALGORITHMS 3

/* function prototypes */
typedef void (*submit_func)(struct rcb*);
typedef struct rcb* (*get_next_func)(void);
extern void scheduler_init(char* selected_algorithm);
extern void scheduler_submit(struct rcb* request_control_block);
extern struct rcb* scheduler_get_next();

/* scheduler struct */
struct scheduler_info {
    char* algorithm;                 /* name of the algorithm (e.g. SJF) */
    submit_func submit;         /* pointer to submit function */
    get_next_func get_next;     /* pointer to get_next function */
};
