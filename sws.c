/*
 * File: sws.c
 * Author: Alex Brodsky
 * Purpose: This file contains the implementation of a simple web server.
 *          It consists of two functions: main() which contains the main
 *          loop accept client connections, and serve_client(), which
 *          processes each client request.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network.h"

/* constants */
#define MAX_HTTP_SIZE 8192                 /* size of buffer to allocate */
#define MAX_REQUESTS 100                   /* max # of requests          */
#define TRUE 1
#define FALSE 0


/* request control block */
struct rcb {
    int sequence_number;         /* Sequence number of the request                         */
    int client_file_descriptor;  /* File descriptor of the client                          */
    int bytes_remaining;         /* # of bytes that remain to be sent                      */
    int quantum;                 /* Max # of bytes to be sent when the request is serviced */
    FILE* file;                  /* File handle of the requested file                      */
    struct rcb* next_block;             /* Pointer to the next block in line                      */
};


/* global variables */
static struct rcb request_table[MAX_REQUESTS];
static struct rcb *free_rcb;

/**
 * Function that initializes the request control block
 */
void rcb_init() {
    free_rcb = request_table;
    //init sequence numbers in the request table:
    for (int i = 0; i < MAX_REQUESTS - 1; i++) {
        request_table[i].sequence_number = i+1;
        request_table[i].next_block = &request_table[i+1];
    }
}

/**
 * Function that creats and allocates a new request control block
 */
struct rcb *rcb_alloc(void) {
    struct rcb *block;

    //TODO: Mutex lock for multithreading
    block = free_rcb;                       // allocate
    free_rcb = free_rcb->next_block;

    //set block's memory location to 0's to make space for the new block:
    memset(block, 0, sizeof(struct rcb));

    return block;
}

/**
 * Function that frees the request control block
 */
void rcb_free(struct rcb *block) {
    //TODO: Mutex lock for multithreading
    block->next_block = free_rcb;
    free_rcb = block;
}


/* This function takes a file handle to a client, reads in the request,
 *    parses the request, and sends back the requested file.  If the
 *    request is improper or the file is not available, the appropriate
 *    error is sent back.
 * Parameters:
 *             fd : the file descriptor to the client connection
 * Returns: None
 */
static void serve_client( int fd ) {
  static char *buffer;                              /* request buffer */
  char *req = NULL;                                 /* ptr to req file */
  char *brk;                                        /* state used by strtok */
  char *tmp;                                        /* error checking ptr */
  FILE *fin;                                        /* input file handle */
  int len;                                          /* length of data read */

  if( !buffer ) {                                   /* 1st time, alloc buffer */
    buffer = malloc( MAX_HTTP_SIZE );
    if( !buffer ) {                                 /* error check */
      perror( "Error while allocating memory" );
      abort();
    }
  }

  memset( buffer, 0, MAX_HTTP_SIZE );
  if( read( fd, buffer, MAX_HTTP_SIZE ) <= 0 ) {    /* read req from client */
    perror( "Error while reading request" );
    abort();
  }

  /* standard requests are of the form
   *   GET /foo/bar/qux.html HTTP/1.1
   * We want the second token (the file path).
   */
  tmp = strtok_r( buffer, " ", &brk );              /* parse request */
  if( tmp && !strcmp( "GET", tmp ) ) {
    req = strtok_r( NULL, " ", &brk );
  }

  if( !req ) {                                      /* is req valid? */
    len = sprintf( buffer, "HTTP/1.1 400 Bad request\n\n" );
    write( fd, buffer, len );                       /* if not, send err */
  } else {                                          /* if so, open file */
    req++;                                          /* skip leading / */
    fin = fopen( req, "r" );                        /* open file */
    if( !fin ) {                                    /* check if successful */
      len = sprintf( buffer, "HTTP/1.1 404 File not found\n\n" );
      write( fd, buffer, len );                     /* if not, send err */
    } else {                                        /* if so, send file */
      len = sprintf( buffer, "HTTP/1.1 200 OK\n\n" );/* send success code */
      write( fd, buffer, len );

      do {                                          /* loop, read & send file */
        len = fread( buffer, 1, MAX_HTTP_SIZE, fin );  /* read file chunk */
        if( len < 0 ) {                             /* check for errors */
            perror( "Error while writing to client" );
        } else if( len > 0 ) {                      /* if none, send chunk */
          len = write( fd, buffer, len );
          if( len < 1 ) {                           /* check for errors */
            perror( "Error while writing to client" );
          }
        }
      } while( len == MAX_HTTP_SIZE );              /* the last chunk < 8192 */
      fclose( fin );
    }
  }
  close( fd );                                     /* close client connection*/
}


/* This function is where the program starts running.
 *    The function first parses its command line parameters to determine port #
 *    Then, it initializes, the network and enters the main loop.
 *    The main loop waits for a client (1 or more to connect, and then processes
 *    all clients by calling the seve_client() function for each one.
 * Parameters:
 *             argc : number of command line parameters (including program name
 *             argv : array of pointers to command line parameters
 * Returns: an integer status code, 0 for success, something else for error.
 */
int main(int argc, char **argv) {
    int port = -1;                                    /* server port # */
    int fd;                                           /* client file descriptor */
    int numThreads;
    int cacheSize;
    struct rcb *request;

    /* check for and process input parameters: */
    if((argc <= 4) || (sscanf(argv[1], "%d", &port) < 1)
                   || (sscanf(argv[3], "%d", &numThreads) < 1)
                   || (sscanf(argv[4], "%d", &cacheSize) < 1)) {
        printf("usage: sws <port> <schedulerAlgorithm> <numThreads> <cacheSize>\n");
        return 0;
    }

    /* initialize all components: */
    network_init(port);                /* init network module */
    rcb_init();                        /* init rcb */



    for( ;; ) {                                                  /* main loop */
        network_wait();                                          /* wait for clients */

        for(fd = network_open(); fd >= 0; fd = network_open()) { /* get clients */
            request = rcb_alloc();
            request->client_file_descriptor = fd;
            serve_client(fd);                                  /* process each client */
        }
    }
}
