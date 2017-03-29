#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "network.h"
#include "scheduler.h"

/* constants */
#define MAX_HTTP_SIZE 8192   /* size of buffer to allocate */
#define MAX_REQS 100         /* max # of requests */
#define TRUE 1
#define FALSE 0

/* global variables */
static struct rcb requests[MAX_REQS];      /* request table */
static struct rcb* free_rcb;
static int request_counter = 1;                   /* request counter */


/* This function takes a file handle to a client, reads in the request,
 *    parses the request, and sends back the requested file.  If the
 *    request is improper or the file is not available, the appropriate
 *    error is sent back.
 * Parameters:
 *             fd : the file descriptor to the client connection
 * Returns: None
 */
static struct rcb* serve_client(int fd) {
    static char *buffer;                              /* request buffer */
    struct stat st;                                   /* struct for file size */
    char *req = NULL;                                 /* ptr to req file */
    char *brk;                                        /* state used by strtok */
    char *tmp;                                        /* error checking ptr */
    FILE *fin;                                        /* input file handle */
    int len = 0;                                      /* length of data read */
    int left = MAX_HTTP_SIZE;                         /* amount of buffer left */
    struct rcb* r;                                           /* new rcb */

    if(!buffer) {                                   /* 1st time, alloc buffer */
        buffer = malloc(MAX_HTTP_SIZE);
        if(!buffer) {                                 /* error check */
            perror( "Error while allocating memory" );
            abort();
        }
    }

    memset(buffer, 0, MAX_HTTP_SIZE);
    for(tmp = buffer; !strchr(tmp, '\n'); left -= len) { /* read req line */
        tmp += len;
        len = read(fd, tmp, left);                    /* read req from client */
        if(len <= 0) {                                /* if read incomplete */
            perror("Error while reading request");      /* no need to go on */
            close(fd);
            return NULL;
        }
    }

    /* standard requests are of the form
    *   GET /foo/bar/qux.html HTTP/1.1
    * We want the second token (the file path).
    */
    tmp = strtok_r(buffer, " ", &brk);              /* parse request */
    if(tmp && !strcmp("GET", tmp)) {
        req = strtok_r(NULL, " ", &brk);
    }

    if(!req) {                                      /* is req valid? */
        len = sprintf(buffer, "HTTP/1.1 400 Bad request\n\n");
        write(fd, buffer, len);                       /* if not, send err */
    } else {                                          /* if so, open file */
        req++;                                          /* skip leading / */
        fin = fopen(req, "r");                        /* open file */
        if(!fin) {                                    /* check if successful */
            len = sprintf(buffer, "HTTP/1.1 404 File not found\n\n");
            write(fd, buffer, len);                     /* if not, send err */
        } else if(!fstat( fileno( fin ), &st)) {     /* if so, start request */
            len = sprintf(buffer, "HTTP/1.1 200 OK\n\n");/* send success code */
            write(fd, buffer, len);

            r = free_rcb;                                 /* allocate RCB */
            free_rcb = free_rcb->next_rcb;
            memset(r, 0, sizeof(struct rcb));

            r->sequence_number = request_counter++;                          /* init RCB */
            r->client_file_descriptor = fd;
            r->file = fin;
            r->bytes_remaining = st.st_size;
            return r;                                     /* return rcb */
        }
        fclose(fin);
    }
    close(fd);                                     /* close client connectuin*/
    return NULL;                                     /* not a valid request */
}


/* This function takes a file handle to a client, reads in the request,
 *    parses the request, and sends back the requested file.  If the
 *    request is improper or the file is not available, the appropriate
 *    error is sent back.
 * Parameters:
 *             fd : the file descriptor to the client connection
 * Returns: None
 */
static int serve(struct rcb* req) {
    static char* buffer;                              /* request buffer */
    int len;                                          /* length of data read */
    int n;                                            /* amount to send */

    if (!buffer) {                                    /* 1st time, alloc buffer */
        buffer = malloc(MAX_HTTP_SIZE);
        if (!buffer) {                                /* error check */
            perror("Error while allocating memory");
            abort();
        }
    }

    n = req->bytes_remaining;                                     /* compute send amount */
    if (!n) {                                          /* if 0, we're done */
        return 0;
    } else if (req->bytes_max_allowed && (req->bytes_max_allowed < n)) {           /* if there is limit */
        n = req->bytes_max_allowed;                                  /* send upto the limit */
    }
    req->bytes_last_sent = n;                                     /* remember send size */

    do {                                               /* loop, read & send file */
        len = n < MAX_HTTP_SIZE ? n : MAX_HTTP_SIZE;   /* how much to read */
        len = fread(buffer, 1, len, req->file);        /* read file chunk */
        if(len < 1) {                                  /* check for errors */
            perror("Error while reading file");
            return 0;
        } else if (len > 0) {                          /* if none, send chunk */
            len = write(req->client_file_descriptor, buffer, len);
            if (len < 1) {                             /* check for errors */
                perror("Error while writing to client");
                return 0;
            }
            req->bytes_remaining -= len;                          /* reduce what remains */
            n -= len;
        }
    } while((n > 0) && (len == MAX_HTTP_SIZE));        /* the last chunk < 8192 */

    return req->bytes_remaining > 0;                              /* return true if not done */
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
int main( int argc, char **argv ) {
    int port = -1;                                   /* server port # */
    int fd;                                          /* client file descriptor */
    int numThreads;
    struct rcb* request;                             /* next request to process */

    /* check for and process input parameters: */
    if((argc <= 3) || (sscanf(argv[1], "%d", &port) < 1)
                   || (sscanf(argv[3], "%d", &numThreads) < 1)) {
        printf("usage: sws <port> <schedulerAlgorithm> <numThreads>\n");
        printf("For example: ./sws 8080 SJF 100\n");
        return 0;
    }

    /* init all components */
    scheduler_init(argv[2]);   //argv[2] is the char* scheduler algorithm
    network_init(port);

    free_rcb = requests;
    for (int i = 0; i < MAX_REQS-1; i++) {
        requests[i].next_rcb = &requests[i+1];
    }

    /* infinite loop */
    while (TRUE) {
        network_wait(); //wait for clients
        do {
            for (fd = network_open(); fd >= 0; fd = network_open()) {
                request = serve_client(fd);
                if (request) {
                    scheduler_submit(request);
                }
            }
            request = scheduler_get_next();
            if (request && serve(request)) {
                scheduler_submit(request);
            } else if (request) {
                request->next_rcb = free_rcb;
                free_rcb = request;
                fclose(request->file);
                close(request->client_file_descriptor);
                printf("Request %d completed.\n", request->sequence_number);
                fflush(stdout);
            }
        } while(request);
    }
}
