/******************************************************************************
 * @Title: multi-lookup.c
 *
 * @author(s) Brandon Lewien
 * @date March 24, 2019
 * @version 2.7
 *
 * Compiled using CU CS Virtual Machine Fall 2018 Linux Terminal
 *
 * Description:
 * This file contains functions for a thread-safe multithreading DNS Lookup
 *
 * ***************************************************************************/

#include "multi-lookup.h"

/*
 * Function: requester_multithread
 * -------------------------------
 *   Input(s) : Parameter pass in structure for a secure way to variable share
 *   Output(s): serviced.txt file
 *              From reading file hostname to queue
 *   Return   : NULL
 *   Purpose  : This allows for a round robin style multithread read of (a) file(s)
 */
void * requester_multithread(void * input) {
    Parameters * context = input;
    int num_files_ser    = 0;
    int rr               = context->thread_id % context->number_files;
    int finish           = false;
    bool counter_done    = false;

    pthread_t real_tid   = pthread_self();
    FILE * input_file[MAX_INPUT_FILES] = {[0 ... MAX_INPUT_FILES-1] = NULL};
    bool done[MAX_INPUT_FILES]         = {[0 ... MAX_INPUT_FILES-1] = false};

    for (int i = 0; i < context->number_files; ++i) {
        input_file[i] = context->infile[i];
        done[i] = context->done[i];
    }
    char hostname[MAX_NAME_LENGTH];
    char * temp;
    while (!finish) {
        finish = true;
        while (done[rr] != true) {
            pthread_mutex_lock(context->reading);                   // Since fscanf is threadsafe, this simulates as if it isn't
            if ((input_file[rr] != NULL) && (fscanf(input_file[rr], INPUTFS, hostname) > 0)) {
                pthread_mutex_unlock(context->reading); 
                temp = malloc(sizeof(char[MAX_NAME_LENGTH]));
                strcpy(temp, hostname); 
                pthread_mutex_lock(context->bufferM); 
                while(queue_is_full(context->q)) {
                    pthread_mutex_unlock(context->bufferM);
                    usleep((rand()%101));                           // Weirdly enough this speeds up compute time
                    pthread_mutex_lock(context->bufferM);
                }
                queue_push(context->q, temp);
                pthread_mutex_unlock(context->bufferM); 
                counter_done = true;
                pthread_mutex_lock(context->reading); 
            }
            else done[rr] = true;                                   // Specific file finished servicing flag
            pthread_mutex_unlock(context->reading); 
        }
        if (counter_done == true) {
            num_files_ser++;                                        // Increment number of files that thread serviced
            counter_done = false;
        }
        for (int i = 0; i < context->number_files; ++i) 
            if (done[i] == false) finish = false;                   // All files are not done
        rr = (rr + 1) % context->number_files;                      // Round Robin style
    }   
    pthread_mutex_lock(context->writeM);
    ++(*(context->signstate));
    fprintf(context->outfile, "Thread %d,%lu serviced %d files.\n", (context->thread_id) + 1, real_tid, num_files_ser);
    pthread_mutex_unlock(context->writeM);
    return NULL;
}
/*
 * Function: resolver
 * ------------------
 *   Input(s) : Parameter pass in structure for a secure way to variable share
 *   Output(s): results.txt file
 *   Return   : NULL
 *   Purpose  : This allows for a multithread read of a queue and write of a file
 */
void * resolver(void * input) {
    Parameters * context = input;
    char * hostname;
    char ipstr[MAX_IP_LENGTH];
    void queueResolve() {
        pthread_mutex_lock(context->bufferM);
        while(!queue_is_empty(context->q)) {
            hostname = queue_pop(context->q);
            pthread_mutex_unlock(context->bufferM);
            if(dnslookup(hostname, ipstr, MAX_IP_LENGTH) == UTIL_FAILURE) {
                fprintf(stderr, "dnslookup error: %s\n", hostname); 
                strncpy(ipstr, "", MAX_IP_LENGTH);
            }
            pthread_mutex_lock(context->write2M);
            fprintf(context->outfile, "%s, %s\n", hostname, ipstr);
            pthread_mutex_unlock(context->write2M);
            free(hostname); 
            pthread_mutex_lock(context->bufferM);
        }
        pthread_mutex_unlock(context->bufferM);
    }
    while((*(context->signstate)) < (context->wait)) { 
        queueResolve();
    }
    queueResolve(); 
    return NULL;
}

int main(int argc, char *argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    long long millisecondStart = start.tv_sec * 1000LL + start.tv_usec / 1000;

    if (argc < MINARG)  {
        fprintf(stderr, "Not enough arguments\n");
        return FAILED;
    }
    int number_files = argc - 5;
    if (number_files > MAX_INPUT_FILES) {
        fprintf(stderr, "Too many files to service\n");
        return FAILED;
    }
    if (atoi(argv[1]) > MAX_REQUESTER_THREADS) {
        fprintf(stderr, "Too many optimal requester threads\n");
        return FAILED;
    }
    if (atoi(argv[2]) > MAX_RESOLVER_THREADS) {
        fprintf(stderr, "Too many optimal resolver threads\n");
        return FAILED;
    }

    FILE * infile[MAX_INPUT_FILES];
    FILE * servicedf   = NULL;
    FILE * resultsfile = NULL;

    pthread_t request_list[MAX_REQUESTER_THREADS];
    pthread_t resolve_list[MAX_RESOLVER_THREADS];
    pthread_mutex_t bufferM;
    pthread_mutex_t writeM;
    pthread_mutex_t reading;
    pthread_mutex_t write2M;
    pthread_mutex_init(&bufferM, NULL);
    pthread_mutex_init(&writeM , NULL);
    pthread_mutex_init(&reading, NULL);
    pthread_mutex_init(&write2M, NULL);

    Parameters reqstruct[MAX_REQUESTER_THREADS];
    Parameters resstruct[MAX_RESOLVER_THREADS];

    int number_threads = 0;
    int sp_numfiles    = number_files;

    int signstate      = 0;
    bool no_file_flag[MAX_INPUT_FILES]  = {[0 ... MAX_INPUT_FILES-1] = false};
    bool done[MAX_INPUT_FILES]          = {[0 ... MAX_INPUT_FILES-1] = false};

    queue q;
    queue_init(&q, QUEUE_SIZE);

    void cleanup(void) {
        queue_cleanup(&q);
        pthread_mutex_destroy(&bufferM);
        pthread_mutex_destroy(&writeM);
        pthread_mutex_destroy(&reading);
        pthread_mutex_destroy(&write2M);

        for(int i = 0; i < number_files; ++i) {                        
            if(infile[i] && !no_file_flag[i]) {                        // Handle special file false case
                fclose(infile[i]);
            }                           
        }
        if(servicedf) fclose(servicedf); 
        if(resultsfile) fclose(resultsfile);
    } 

    if ((servicedf = fopen(argv[3], "w")) == NULL) {
        fprintf(stderr, "Error: incorrect output file path\n");
        cleanup(); 
        return FAILED;
    }
    if ((resultsfile = fopen(argv[4], "w")) == NULL) {
        fprintf(stderr, "Error: incorrect output file path\n");
        cleanup(); 
        return FAILED;
    }

    for (int i = 0; i < number_files; ++i) {
        infile[i] = fopen(argv[5 + i], "r");
        if (!infile[i]) {
            fprintf(stderr, "Error: incorrect input file path Error File: %s\n", argv[i+5]);
            --sp_numfiles;
            no_file_flag[i] = true;
        }
    }
    for (number_threads = 0; number_threads < atoi(argv[1]); number_threads++) {
        reqstruct[number_threads].signstate    = &signstate;        // Pthread Requester Thread Internal Finish Counter 
        reqstruct[number_threads].outfile      = servicedf;         // Requester serviced output file (write)
        reqstruct[number_threads].number_files = sp_numfiles;       // Requester # file counter (read)
        reqstruct[number_threads].q            = &q;                // Standard pass in queue
        reqstruct[number_threads].bufferM      = &bufferM;          // Requester and Resolver shared mutex
        reqstruct[number_threads].writeM       = &writeM;           // Requester specific mutex
        reqstruct[number_threads].reading      = &reading;          // Requester specific mutex
        reqstruct[number_threads].thread_id    = number_threads;    // gettid() doesn't work
        memcpy(reqstruct[number_threads].infile, infile, sizeof(FILE *) * MAX_INPUT_FILES);      
        memcpy(reqstruct[number_threads].done, done, sizeof(bool) * MAX_INPUT_FILES);        
        if ( 0 != pthread_create(&request_list[number_threads], NULL, requester_multithread, (void *)&reqstruct[number_threads])) {
            fprintf(stderr, "Requester Pthread create failed - Error Thread: %d\n", number_threads);
            cleanup(); 
            return FAILED;
        }
    }

    for (int i = 0; i < atoi(argv[2]); ++i) {
        resstruct[i].signstate = &signstate;                        // Pthread Requester Thread Internal Finish Counter                 
        resstruct[i].outfile   = resultsfile;                       // Resolver results file (write)
        resstruct[i].q         = &q;                                // Standard pass in queue
        resstruct[i].bufferM   = &bufferM;                          // Requester and Resolver shared mutex
        resstruct[i].write2M   = &write2M;                          // Resolver specific mutex
        resstruct[i].wait      = number_threads;                    // Pthread Requester Thread External Finish Counter 
        if(0 != pthread_create(&resolve_list[i], NULL, resolver, (void *)&resstruct[i])) {
            fprintf(stderr, "Resolver Pthread create failed - Error Thread: %d\n", i);
            cleanup(); 
            return FAILED;
        }          
    }

    for (int i = 0; i < atoi(argv[1]); ++i) {
        pthread_join(request_list[i], NULL);                        // Wait for all requester threads to complete
    }
    for (int i = 0; i < atoi(argv[2]); ++i) {
        pthread_join(resolve_list[i], NULL);                        // Wait for all resolver threads to complete
    }

    cleanup();                                                      // Standard no fail cleanup routine

    gettimeofday(&end, NULL);
    long long millisecondEnd = end.tv_sec * 1000LL + end.tv_usec / 1000;
    fprintf(stderr, "Total runtime: %lld milliseconds\n",millisecondEnd-millisecondStart);
    return 0;
}