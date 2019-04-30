/******************************************************************************
 * @Title: pager-lru.c
 *
 * @author Brandon Lewien
 * @date 4/14/2019
 * @version 1.0
 *
 * Compiled using Linux
 *
 * Description:
 * This file does paging with a Least Recently Used Algorithm
 * The jist is if we run out of space to insert a new page (pagein),
 * we scan the already inputted pages for the one that was 
 * used the least and perform a swap (pageout).
 * Define PREDICTPRINTOUT and use reset; make; ./test-lru to get trace
 * Additional credit goes to Dr. Alva Couch and Andy Sayler
 * ***************************************************************************/

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

#ifdef PREDICTPRINTOUT
int memory[MAXPROCESSES];
#endif

void pageit(Pentry q[MAXPROCESSES]) { 

    static int initialized = 0;
    static int tick = 1;                                // Artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    int page = 0;
    int ret  = 0;
    if(!initialized){                                   // Initialize timestamps on first run
        for(int proctmp=0; proctmp < MAXPROCESSES; ++proctmp) {
            for(int pagetmp=0; pagetmp < MAXPROCPAGES; ++pagetmp) {
                timestamps[proctmp][pagetmp] = 0; 
            }
        }
        initialized = 1;
    }
    
        for(int process = 0; process < MAXPROCESSES; ++process) { 
                                                        // Is process active?
        if(q[process].active) {                         // Dedicate all work to first active process 
            page = q[process].pc / PAGESIZE;            // Page the program counter needs
            timestamps[process][page] = tick;           // Corresponding process and page stores tick
            if(!q[process].pages[page]) {               // Is page swapped out?
                                                        // Try to swap in
                if(!pagein(process,page)) {             // If swapping fails, swap out another page 
                    int minimum = tick + 1;             // Set min to max for start
                    for(int oldpage = 0; oldpage < q[process].npages; ++oldpage) {
                        if(q[process].pages[oldpage] == 1) {
                            if(timestamps[process][oldpage] < minimum) {
                                minimum = timestamps[process][oldpage];
                                ret = oldpage;
                            }
                        }
                    }
#ifdef PREDICTPRINTOUT
                    if(memory[process] != page) { 
                        memory[process] = page;         
                        fprintf(stderr, "process %d, page %d, oldpageret %d BUFFER\n", process, page, ret); 
                        pageout(process, ret);          // Try to swap in
                    }

#else
                        pageout(process, ret);          // Try to swap in
#endif
                }
            }
        }        
    }
    tick++;                                             // Advance time for next pageit iteration
} 