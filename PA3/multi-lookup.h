#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"
#include "util.h"
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define MAX_INPUT_FILES         10
#define MAX_RESOLVER_THREADS    10
#define MAX_REQUESTER_THREADS   10
#define MAX_NAME_LENGTH       1025      // Take to account the NULL 
#define MINARG                   5      // #Threads, #Threads, #Output, #Output, #Input
#define INPUTFS           "%1024s"  
#define FAILED                  -1
#define MAX_IP_LENGTH           46      // INET6_ADDRSTRLEN == 46
#define QUEUE_SIZE              10


typedef struct Param_s{
	FILE * infile[MAX_INPUT_FILES];     // Requester files (read)
	FILE * outfile;                     // Requester & Resolver files (write)
	queue * q;                          // Standard pass in queue
	int wait;                           // Pthread Requester Thread External Finish Counter
	int input_checker[MAX_INPUT_FILES]; // Skipper for inputs if input file is invalid 
	int number_files;                   // Requester # file counter (read)
	int thread_id;                      // gettid() doesn't work
	int * signstate;                    // Pthread Requester Thread Internal Finish Counter 
	bool done[MAX_INPUT_FILES];         // Checker to see if all input read files are done
	pthread_mutex_t * bufferM;          // Requester and Resolver shared mutex
	pthread_mutex_t * writeM;           // Requester specific mutex
	pthread_mutex_t * reading;          // Requester specific mutex
	pthread_mutex_t * write2M;          // Resolver specific mutex
} Parameters;


void* requester_multithread(void * input);
void* resolver(void * input);
int main(int argc, char *argv[]);

#endif