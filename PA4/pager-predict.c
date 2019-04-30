/******************************************************************************
 * @Title: pager-predict.c
 *
 * @author Brandon Lewien
 * @date 4/14/2019
 * @version 1.0
 *
 * Compiled using Linux
 *
 * Description:
 * Run pager-lru.c with PREDICTPRINTOUT defined
 * Run in terminal reset; make; ./test-lru
 * Copy everything from terminal
 * With an editor, sort by processes (Process 0, Page blah\n Process 0, Page blah) 
 * etc. This allows all processes to be sorted so we can predict what comes next.
 * Search for the lines with specific pages (Pull all page 0's from all processes) 
 * Take 2 next lines by multiselecting multiple lines where the search found the 
 * specific page. From here, you can see different processes but the trend of what 
 * comes after. This allows us to have a basic trend of what comes next with the 
 * LRU algorithm. From here, we can see a common pattern. Usually the prediction 
 * sees the page+1 and page+2 as a next pages needed to be paged in.
 * The majority of the steps after this is guess and check. But almost always we 
 * need to include the page+1 and +2 pages for a prediction besides for pages
 * after 11.
 * Additional credit goes to Dr. Alva Couch and Andy Sayler
 * ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    int page, process;
    for(process = 0; process < MAXPROCESSES; ++process) { 
        if(q[process].active) {
            int prediction[15] = {0};
            page = q[process].pc / PAGESIZE;        // Page the program counter needs
            prediction[page] = 1;                   // Set the current page prediction to true
            switch(page) {           
                case(2):
                    prediction[0] = 1;
                    prediction[3] = 1;
                    prediction[4] = 1;
                    break;              
                case(3):
                    prediction[0]  = 1;
                    prediction[4]  = 1;
                    prediction[10] = 1;
                    break;
                case(7):
                    prediction[0] = 1;
                    prediction[8] = 1;
                    prediction[9] = 1;
                    break;
                case(8):
                    prediction[0]  = 1;
                    prediction[9]  = 1;
                    prediction[10] = 1;
                    break;
                case(10):
                    prediction[0]  = 1;
                    prediction[11] = 1;
                    prediction[12] = 1;
                    break;
                case(11):
                    prediction[0]  = 1;
                    prediction[1]  = 1;
                    prediction[12] = 1;
                    break;
                case(12):
                    prediction[0]  = 1;
                    prediction[9]  = 1;
                    prediction[13] = 1;
                    break;
                case(13):
                    prediction[0]  = 1;
                    prediction[9]  = 1;
                    prediction[10] = 1;
                    prediction[14] = 1;
                    break;
                case(14):
                    prediction[0] = 1;
                    prediction[2] = 1;
                    break;
                default:
                    prediction[page + 1] = 1;         // Most common case
                    prediction[page + 2] = 1;         // Second most common case
            }
            for(int i = 0; i < 15; ++i) {
                if(prediction[i] == 0) {
                    pageout(process, i);            // Page out those that won't be used
                }
                if(prediction[i] == 1) {
                    pagein(process, i);             // Page in those that will be used
                }
            }
        }
    }
} 
