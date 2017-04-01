#include <stdio.h>     //provides I/O functions
#include <stdlib.h>    //provides general C functions
#include <string.h>    //provides functions for char[] manipulation
#include <unistd.h>    //provides access to the POSIX API
#include <sys/stat.h>  //needed to gather info about file attributes
#include <pthread.h>   //multithreading library
#include <stdarg.h>    //unknown function arguments library

#include "network.h"
#include "scheduler.h"
#include "scheduler_queue.h"

/* constants */
#define MAX_HTTP_SIZE 8192
#define MAX_REQUESTS 100
#define TRUE 1
#define FALSE 0

/* global variables */
static struct rcb request_table[MAX_REQUESTS];
static struct rcb* free_rcb;
static struct scheduler_queue work_queue;
static int request_counter = 1;
pthread_mutex_t alloc_rcb_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t rcb_available = PTHREAD_COND_INITIALIZER;

/*
 * Function to safely print processing messages with multithreading.
 */
void thread_print_function( char * input_string, ... ) {
    static pthread_mutex_t thread_print_lock = PTHREAD_MUTEX_INITIALIZER;

    //get and format parameter arguments for printing
    va_list parameter_list;
    va_start(parameter_list, input_string);

    //lock, print, flush, unlock
    pthread_mutex_lock( &thread_print_lock );
    vprintf(input_string, parameter_list);
    fflush(stdout);
    pthread_mutex_unlock(&thread_print_lock);
}


/* This function takes a file handle to a client, reads in the request,
 * parses the request, and sends back the requested file.  If the
 * request is improper or the file is not available, the appropriate
 * error is sent back.
 */
static struct rcb* serve_client(struct rcb* request_block) {
    static char* buffer;
    struct stat st;  //struct for file size
    char* request_file_ptr = NULL;
    char* strtok_result;
    char* temp_bytes;
    FILE* input_file;
    int num_bytes_to_read = 0;
    int buffer_left = MAX_HTTP_SIZE;
    int file_descriptor = request_block->client_file_descriptor;

    //allocate the request buffer if it has not yet been allocated:
    if (!buffer) {
        buffer = malloc(MAX_HTTP_SIZE);
        if (!buffer) {
            perror("Error while allocating memory");
            abort();
        }
    }

    memset(buffer, 0, MAX_HTTP_SIZE);

    //read the request line:
    for(temp_bytes = buffer; !strchr(temp_bytes, '\n'); buffer_left -= num_bytes_to_read) {
        temp_bytes += num_bytes_to_read;
        //read client request:
        num_bytes_to_read = read(file_descriptor, temp_bytes, buffer_left);

        //check if the read is complete:
        if(num_bytes_to_read <= 0) {
            perror("Error while reading request");
            return NULL;
        }
    }

    //get the file path (2nd token of the format: GET /foo/bar/qux.html HTTP/1.1)
    temp_bytes = strtok_r(buffer, " ", &strtok_result);

    //parse the request:
    if(temp_bytes && !strcmp("GET", temp_bytes)) {
        request_file_ptr = strtok_r(NULL, " ", &strtok_result);
    }

    //check if the request is valid:
    if(!request_file_ptr) {
        //invalid request. Write the error:
        num_bytes_to_read = sprintf(buffer, "HTTP/1.1 400 Bad request\n\n");
        write(file_descriptor, buffer, num_bytes_to_read);
    } else {
        //request is valid.

        // get file path for later use
        strncpy(request_block->file_path, request_file_ptr, FILENAME_MAX);

        //Open the file:
        request_file_ptr++;   //skip leading /
        input_file = fopen(request_file_ptr, "r");

        //check if the file was opened:
        if (!input_file) {
            //file was not opened. Write the error:
            num_bytes_to_read = sprintf(buffer, "HTTP/1.1 404 File not found\n\n");
            write(file_descriptor, buffer, num_bytes_to_read);
        } else if (!fstat(fileno(input_file), &st)) {
            //file was opened. Send the success message and start the request:
            num_bytes_to_read = sprintf(buffer, "HTTP/1.1 200 OK\n\n");
            write(file_descriptor, buffer, num_bytes_to_read);

            // //allocate the RCB:
            // request_block = free_rcb;
            // free_rcb = free_rcb->next_rcb;
            // memset(request_block, 0, sizeof(struct rcb));

            //initialize RCB values and return it:
            request_block->sequence_number = request_counter++;
            request_block->client_file_descriptor = file_descriptor;
            request_block->file = input_file;
            request_block->bytes_remaining = st.st_size;

            return request_block;
        }
        fclose(input_file);
    }
    //invalid request. Close the client connection:
    close(file_descriptor);
    return NULL;
}


/*
 * Determine if the request is finished yet.
 */
static int serve(struct rcb* request_block) {
    static char* buffer;
    int num_bytes_to_read;
    int num_bytes_to_send;

    //allocate the request buffer if it has not yet been allocated:
    if (!buffer) {
        buffer = malloc(MAX_HTTP_SIZE);
        if (!buffer) {
            perror("Error while allocating memory");
            abort();
        }
    }

    //get the # of bytes to send:
    num_bytes_to_send = request_block->bytes_remaining;

    //check how much to send and if there is a size cap:
    if (!num_bytes_to_send) {
        //request is finished.
        return 0;
    } else if (request_block->bytes_max_allowed && (request_block->bytes_max_allowed < num_bytes_to_send)) {
        //there is a size cap. Set the size cap:
        num_bytes_to_send = request_block->bytes_max_allowed;
    }

    //store the amount of data we are sending:
    request_block->bytes_last_sent = num_bytes_to_send;

    //read and send the file while there's still bytes to send:
    do {
        if (num_bytes_to_send < MAX_HTTP_SIZE) {
            num_bytes_to_read = num_bytes_to_send;
        } else {
            //cap read amount to the max size:
            num_bytes_to_read = MAX_HTTP_SIZE;
        }
        //read the file:
        num_bytes_to_read = fread(buffer, 1, num_bytes_to_read, request_block->file);

        //prepare to write to the client:
        if(num_bytes_to_read < 1) {
            perror("Error while reading file");
            return 0;
        } else if (num_bytes_to_read > 0) {
            //write to the client:
            num_bytes_to_read = write(request_block->client_file_descriptor, buffer, num_bytes_to_read);
            if (num_bytes_to_read < 1) {
                perror("Error while writing to client");
                return 0;
            }

            //set remaining bytes to send:
            request_block->bytes_remaining -= num_bytes_to_read;
            num_bytes_to_send -= num_bytes_to_read;
        }
    } while((num_bytes_to_send > 0) && (num_bytes_to_read == MAX_HTTP_SIZE));

    //return true if there are still bytes remaining to be sent, and false otherwise.
    return request_block->bytes_remaining > 0;
}


/*
 * Each thread executes this function.
 */
static void *thread_execution_function(void* arg) {
    struct rcb* request_block;
    int block = 0;

    //enter infinite loop:
    while (TRUE) {
        //dequeue from the work queue and grab the RCB:
        request_block = scheduler_dequeue(&work_queue, block);
        if (request_block) {
            if (serve_client(request_block)) {

                //print file admitted message and submit to scheduler
                thread_print_function("Request for file %s admitted.\n", request_block->file_path );
                submit_to_scheduler(request_block);

            } else {
                close(request_block->client_file_descriptor);

                //we are enterting a critical section. Lock the state:
                pthread_mutex_lock(&alloc_rcb_lock);

                //free the RCB:
                request_block->next_rcb = free_rcb;
                free_rcb = request_block;

                //emit a signal that the RCB is now available
                pthread_cond_signal(&rcb_available);

                //we are leaving a critical section. Unlock the state:
                pthread_mutex_unlock(&alloc_rcb_lock);
            }
        } else {
            request_block = get_from_scheduler();
            if (request_block && serve(request_block)) {
                //request is not finished yet.
                //Print sent bytes message and re-submit RCB to the scheduler.
                thread_print_function("Sent %d bytes of file %s.\n", request_block->bytes_last_sent, request_block->file_path);
                submit_to_scheduler(request_block);
            } else if (request_block) {
                //request is finished. Close the file and complete the request.

                //print sent bytes message
                thread_print_function("Sent %d bytes of file %s.\n", request_block->bytes_last_sent, request_block->file_path);

                fclose(request_block->file);
                close(request_block->client_file_descriptor);

                //print file complete message
                thread_print_function( "Request for file %s completed.\n", request_block->file_path );
                fflush(stdout);

                //we are enterting a critical section. Lock the state:
                pthread_mutex_lock(&alloc_rcb_lock);

                //free the RCB:
                request_block->next_rcb = free_rcb;
                free_rcb = request_block;

                //emit a signal that the RCB is now available
                pthread_cond_signal(&rcb_available);

                //we are leaving a critical section. Unlock the state:
                pthread_mutex_unlock(&alloc_rcb_lock);
            }
        }
        block = request_block == NULL;
    }
    return NULL;
}


/* This function is where the program starts running.
 * The function first parses its command line parameters to determine port #.
 * Then it initializes the network and enters the main loop.
 * The main loop waits for a client (1 or more) to connect, and then processes all clients.
 */
int main(int argc, char** argv) {
    int port = -1;
    int fd;
    int num_threads;
    struct rcb* request;
    pthread_t thread_id;

    // check for and process input parameters:
    if((argc <= 3) || (sscanf(argv[1], "%d", &port) < 1)
                   || (sscanf(argv[3], "%d", &num_threads) < 1)) {
        printf("usage: sws <port> <schedulerAlgorithm> <numThreads>\n");
        printf("For example: ./sws 8080 SJF 100\n");
        return 0;
    }

    // initialize all components:
    scheduler_init(argv[2]);   //init the scheduler with the user-inputted algorithm
    network_init(port);        //init network with the given port
    queue_init(&work_queue);   //init the work queue

    // initialize request table:
    free_rcb = request_table;
    for (int i = 0; i < MAX_REQUESTS-1; i++) {
        request_table[i].next_rcb = &request_table[i+1];
    }

    // initialize all threads:
    for (int j = 0; j < num_threads; j++) {
        pthread_create(&thread_id, NULL, &thread_execution_function, NULL);
    }

    // enter infinite loop:
    while (TRUE) {
        //wait for clients:
        network_wait();

        //process requests while number of requests is less than 100
        while (request_counter < MAX_REQUESTS) {
            //grab all file descriptors, get its RCB, and submit it to the scheduler:
            for (fd = network_open(); fd >= 0; fd = network_open()) {
                //we are enterting a critical section. Lock the state:
                pthread_mutex_lock(&alloc_rcb_lock);

                //check if the RCB is free'd up:
                if (!free_rcb) {
                    //wait for the availability signal:
                    pthread_cond_wait(&rcb_available, &alloc_rcb_lock);
                }

                //allocate RCB:
                request = free_rcb;
                free_rcb = free_rcb->next_rcb;

                //we are leaving a critical section. Unlock the state:
                pthread_mutex_unlock(&alloc_rcb_lock);

                memset(request, 0, sizeof(struct rcb));

                //set fd and put it onto the queue:
                request->client_file_descriptor = fd;
                scheduler_enqueue(&work_queue, request);
            }
        }
    }
}
