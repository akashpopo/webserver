#ifndef REQUEST_CONTROL_BLOCK
#define REQUEST_CONTROL_BLOCK   /* singleton pattern */

#include <stdio.h>

/* initialize the request control block */
struct rcb {
    int sequence_number;         /* sequence # of request */
    int client_file_descriptor;  /* client file descriptor */
    int bytes_remaining;         /* number of bytes left to send */
    int bytes_max_allowed;       /* maximum allowed send */
    FILE* file;                  /* FILE handle of the requested file */
    int bytes_last_sent;         /* last amount of bytes sent */
    struct rcb* next_rcb;        /* pointer to next rcb in queue */
};

#endif
