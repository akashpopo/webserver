#include "rcb.h"

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
    char* name;                 /* name of scheduler */
    submit_func submit;         /* func ptr to submit func */
    get_next_func get_next;     /* func ptr tp get_next func */
};
