/*
 * Singleton pattern
 *
 * Source:
 * Singleton Pattern in C. (n.d.). Retrieved March 28, 2017,
 * from http://stackoverflow.com/questions/5171661/singleton-pattern-in-c
 */
#ifndef REQUEST_CONTROL_BLOCK
#define REQUEST_CONTROL_BLOCK

#include <stdio.h>

/* initialize the request control block */
struct rcb {
    int sequence_number;          //sequence # of request
    int client_file_descriptor;   // client file descriptor
    int bytes_remaining;          // number of bytes left to send
    int bytes_max_allowed;        // maximum allowed send
    FILE* file;                   // FILE handle of the requested file
    char file_path[FILENAME_MAX]; // path of fileine
    int bytes_last_sent;          // last amount of bytes sent
    struct rcb* next_rcb;         // pointer to next rcb in queue 
};

#endif
