/*******************************************************************************
The assignment 3 for subject 48450 (RTOS) in University of Technology
Sydney(UTS) This is a template of Program_1.c template. Please complete the code
based on the assignment 3 requirement. Assignment 3

------------------------------Program_1.c template------------------------------
*******************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <pthread.h> /* pthread functions and data structures for pipe */
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>  /* standard I/O routines */
#include <stdlib.h> /* for exit() function */
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h> /* for POSIX API */

typedef struct RR_Params {
  // add your variables here

} ThreadParams;

/**
 * This function calculates Round Robin (RR) with a time quantum of 4, writes
 * waiting time and turn-around time to the RR.
 */
void *worker1(void *params) {
  // add your code here
  return NULL;
}

/**
 * Reads the waiting time and turn-around time through the RR and writes to text
 * file.
 */
void *worker2() {
  // add your code here
  return NULL;
}

/**
 * This main function creates named pipe and threads.
 */
int main(void) {
  /* creating a named pipe(RR) with read/write permission */
  // add your code

  /* initialize the parameters */
  // add your code

  /* create threads */
  // add your code

  /* wait for the thread to exit */
  // add your code

  return 0;
}
