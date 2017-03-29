/*
 * File: rcb.h
 * Author: Alex Brodsky
 * Purpose: Defines the Request Control Block for managing requests.
 */

#ifndef RCB_H
#define RCB_H

#include <stdio.h>

struct rcb {
    struct rcb* next;   /* pointer to next rcb in queue */
    int seq;     /* sequence # of request */
    int client;  /* client file descriptor */
    FILE* file;  /* FILE stream of requested file */
    int left;    /* number of bytes left to send */
    int max;     /* maximum allowed send */
    int last;    /* last amount of bytes sent */
};

#endif
