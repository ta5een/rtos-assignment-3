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

/* --- Constants --- */

/** The maximum string length of the output file name. */
#define OUTPUT_FILE_NAME_LEN 100
/** The pipe channel to read data from. */
#define PIPE_READ 0
/** The pipe channel to write data to. */
#define PIPE_WRITE 1

/* --- Global variables --- */

int num_threads = 2; // two threads, one for `worker1` and one for `worker2`
pthread_attr_t attr;

/* --- Structs --- */

/**
 * Thread parameters for the Round Robin scheduler.
 */
typedef struct rr_params_t {
  int pipe_file[2];
  long int time_quantum;
  char output_file[OUTPUT_FILE_NAME_LEN];
} thread_params_t;

/* --- Prototypes --- */

/**
 * This function calculates Round Robin (RR) with a time quantum of 4, writes
 * waiting time and turn-around time to the FIFO.
 */
void *worker1(void *params);

/**
 * Reads the waiting time and turn-around time through the FIFO and writes to
 * text file.
 */
void *worker2(void *params);

/* --- Main code -- */

/**
 * This main function creates named pipe and threads.
 */
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "USAGE: %s time_quantum output_file \n", argv[0]);
    return EXIT_FAILURE;
  }

  pthread_t tids[num_threads]; // two threads
  thread_params_t params;      // thread parameters

  // Create a named pipe (RR) with read/write permission
  int pipe_result = pipe(params.pipe_file);
  if (pipe_result < 0) {
    perror("Failed to create pipe");
    exit(EXIT_FAILURE);
  }

  // Initialize output file parameter
  strncpy(params.output_file, argv[2], OUTPUT_FILE_NAME_LEN);

  // Initialize time quantum parameter with bound-checking
  params.time_quantum = strtol(argv[1], NULL, 10);
  if (params.time_quantum < 1 || errno == ERANGE) {
    perror("Invalid time quantum provided");
    return EXIT_FAILURE;
  }

  // Create first thread for `worker1`
  if (pthread_create(&(tids[0]), &attr, &worker1, (void *)(&params)) != 0) {
    perror("Failed to create first thread");
    exit(EXIT_FAILURE);
  }

  // Create second thread for `worker1`
  if (pthread_create(&(tids[1]), &attr, &worker2, (void *)(&params)) != 0) {
    perror("Failed to create second thread");
    exit(EXIT_FAILURE);
  }

  // Wait for all threads to finish
  // Inspired by the example found in `man pthread_create`
  int thread_join_res, tnum;
  for (tnum = 0; tnum < num_threads; tnum++) {
    thread_join_res = pthread_join(tids[tnum], NULL);
    if (thread_join_res != 0) {
      errno = thread_join_res;
      perror("Failed to join thread");
      exit(EXIT_FAILURE);
    }
  }

  return EXIT_SUCCESS;
}

void *worker1(void *params) {
  // add your code here
  return NULL;
}

void *worker2(void *params) {
  // add your code here
  return NULL;
}
