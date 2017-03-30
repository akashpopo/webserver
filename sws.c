#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "network.h"
#include "scheduler.h"

/* constants */
#define MAX_HTTP_SIZE 8192
#define MAX_REQUESTS 100
#define TRUE 1
#define FALSE 0

/* global variables */
static struct rcb request_table[MAX_REQUESTS];
static struct rcb* free_rcb;
static int request_counter = 1;


/* This function takes a file handle to a client, reads in the request,
 * parses the request, and sends back the requested file.  If the
 * request is improper or the file is not available, the appropriate
 * error is sent back.
 */
static struct rcb* serve_client(int file_descriptor) {
    static char *buffer;
    struct stat st;  //struct for file size
    char *request_file_ptr = NULL;
    char *strtok_result;
    char *temp_bytes;
    FILE *input_file;
    int num_bytes_to_read = 0;
    int buffer_left = MAX_HTTP_SIZE;
    struct rcb* request_block;

    //allocate the request buffer if it has not yet been allocated:
    if(!buffer) {
        buffer = malloc(MAX_HTTP_SIZE);
        if(!buffer) {
            perror( "Error while allocating memory" );
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
            close(file_descriptor);
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
        //request is valid. Open the file:
        request_file_ptr++;   //skip leading /
        input_file = fopen(request_file_ptr, "r");

        //check if the file was opened:
        if (!input_file) {
            //file was not opened. Write the error:
            num_bytes_to_read = sprintf(buffer, "HTTP/1.1 404 File not found\n\n");
            write(file_descriptor, buffer, num_bytes_to_read);
        } else if (!fstat(fileno(input_file), &st)) {
            //file was opened. Send the success code and start the request:
            num_bytes_to_read = sprintf(buffer, "HTTP/1.1 200 OK\n\n");
            write(file_descriptor, buffer, num_bytes_to_read);

            //allocate the RCB:
            request_block = free_rcb;
            free_rcb = free_rcb->next_rcb;
            memset(request_block, 0, sizeof(struct rcb));

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


/* This function is where the program starts running.
 * The function first parses its command line parameters to determine port #
 * Then, it initializes, the network and enters the main loop.
 * The main loop waits for a client (1 or more to connect, and then processes all clients.
 */
int main(int argc, char **argv) {
    int port = -1;
    int fd;
    int numThreads;
    struct rcb* request;

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

    free_rcb = request_table;
    for (int i = 0; i < MAX_REQUESTS-1; i++) { //init the request table
        request_table[i].next_rcb = &request_table[i+1];
    }

    while (TRUE) {
        network_wait(); //wait for clients
        do {
            //grab all file descriptors, get its RCB, and submit it to the scheduler:
            for (fd = network_open(); fd >= 0; fd = network_open()) {
                request = serve_client(fd);
                if (request) {
                    submit_to_scheduler(request);
                }
            }
            request = get_from_scheduler();

            //check if the request has finished
            if (request && serve(request)) {
                //request is not finished yet. Re-submit it to the scheduler.
                submit_to_scheduler(request);
            }
            else if (request) {
                //request is finished. Close the file and complete the request.
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
