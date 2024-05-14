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

/** Set to 1 to turn on DEBUG, otherwise set to 0. */
#define DEBUG 1

/** Two threads, one for `worker1` and one for `worker2`. */
#define NUM_THREADS 2
/** Number of processes to simulate in Round Robin scheduling. */
#define NUM_RR_PROCESSES 7

/** The pipe channel to read data from. */
#define PIPE_READ 0
/** The pipe channel to write data to. */
#define PIPE_WRITE 1
/** The maximum string length of the output file name. */
#define OUTPUT_FILE_NAME_LEN 100

/* --- Global variables --- */

pthread_attr_t attr;

/* --- Structs --- */

/**
 * Input data for each process.
 */
typedef struct rr_input_data_t {
  int __pid;
  /** When does the process arrive (in milliseconds)? */
  int arrival_time;
  /** CPU cycle count for this process to execute (in milliseconds). */
  int burst_time;
} rr_input_data_t;

/**
 * Thread parameters for the Round Robin scheduler.
 */
typedef struct rr_params_t {
  int pipe_file[2];
  long int time_quantum;
  rr_input_data_t rr_input_data[NUM_RR_PROCESSES];
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

/* --- Main code --- */

/**
 * This main function creates named pipe and threads.
 */
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "USAGE: %s time_quantum output_file \n", argv[0]);
    return EXIT_FAILURE;
  }

  pthread_t tids[NUM_THREADS]; // two threads
  thread_params_t params;      // thread parameters

  // Initialize input data for Round Robin scheduling simulation
  rr_input_data_t rr_input_data[NUM_RR_PROCESSES] = {
      {.__pid = 1, .arrival_time = 8, .burst_time = 10},
      {.__pid = 2, .arrival_time = 10, .burst_time = 3},
      {.__pid = 3, .arrival_time = 14, .burst_time = 7},
      {.__pid = 4, .arrival_time = 9, .burst_time = 5},
      {.__pid = 5, .arrival_time = 16, .burst_time = 4},
      {.__pid = 6, .arrival_time = 21, .burst_time = 6},
      {.__pid = 7, .arrival_time = 26, .burst_time = 2},
  };

  for (int i = 0; i < NUM_RR_PROCESSES; i++) {
    params.rr_input_data[i] = rr_input_data[i];
  }

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
  for (tnum = 0; tnum < NUM_THREADS; tnum++) {
    thread_join_res = pthread_join(tids[tnum], NULL);
    if (thread_join_res != 0) {
      errno = thread_join_res;
      perror("Failed to join thread");
      exit(EXIT_FAILURE);
    }
  }

  return EXIT_SUCCESS;
}

typedef struct rr_queue_node_t {
  rr_input_data_t *data;
  struct rr_queue_node_t *next;
} rr_queue_node_t;

typedef struct rr_queue_t {
  rr_queue_node_t *first;
  rr_queue_node_t *last;
} rr_queue_t;

void rr_queue_init(rr_queue_t *queue) {
  queue->first = NULL;
  queue->last = NULL;
}

int rr_queue_is_empty(rr_queue_t *queue) {
  return queue->first == NULL && queue->last == NULL;
}

void rr_queue_enqueue(rr_queue_t *queue, rr_input_data_t *data) {
  rr_queue_node_t *new_node =
      (rr_queue_node_t *)malloc(sizeof(rr_queue_node_t));
  new_node->data = data;
  new_node->next = NULL;

  if (rr_queue_is_empty(queue)) {
    queue->last = new_node;
    queue->first = queue->last;
  } else {
    queue->last->next = new_node;
    queue->last = new_node;
  }
}

rr_input_data_t *rr_queue_dequeue(rr_queue_t *queue) {
  if (rr_queue_is_empty(queue)) {
    return NULL;
  }

  rr_queue_node_t *node_to_remove = queue->first;
  rr_input_data_t *data = node_to_remove->data;

  if (queue->first == queue->last) {
    // If there is only one node left, unset first and last nodes
    queue->last = NULL;
    queue->first = NULL;
  } else {
    // Otherwise, set the first node to the second node (i.e. shift the nodes)
    queue->first = queue->first->next;
  }

  free(node_to_remove);
  return data;
}

void rr_queue_print(rr_queue_t *queue) {
  rr_queue_node_t *peek_node = queue->first;
  while (peek_node != NULL) {
    printf("P%d -> ", peek_node->data->__pid);
    peek_node = peek_node->next;
  }
  printf("*\n");
}

void *worker1(void *params) {
  thread_params_t *p = params;

  // // stdlib function to sort the input data array by the arrival time.
  // qsort(p->rr_input_data, NUM_RR_PROCESSES, sizeof(rr_input_data_t),
  //       sort_by_arrive_time);

  int cycle = 0;
  int curr_inc_quantum = 0;
  // int done = 0;

  rr_queue_t queue;
  rr_queue_init(&queue);
  rr_input_data_t *curr_process = NULL;

  while (true) {
    rr_queue_print(&queue);
    printf("-> Cycle: %d\n", cycle);
    printf("-> Inc Quantum: %d\n", curr_inc_quantum);
    printf("PID\tArrival\tBurst\n");

    // Check if there's any new processes right now
    for (int i = 0; i < NUM_RR_PROCESSES; i++) {
      rr_input_data_t *curr = &p->rr_input_data[i];
      printf("P%d\t%d\t%d", i + 1, curr->arrival_time, curr->burst_time);
      if (curr->arrival_time == cycle) {
        printf("\t-> ARRIVE\n");
        rr_queue_enqueue(&queue, curr);
      } else {
        printf("\n");
      }
    }

    // TODO: Queue approach doesn't work!
    printf("-> Current Process is NULL? %d", curr_process == NULL);
    if (curr_process != NULL) {
    handle_curr_process:
      if (curr_inc_quantum + 1 < p->time_quantum) {
        printf("\t/\tDecrementing P%d", curr_process->__pid);
        curr_inc_quantum++;
        // TODO: Constrain decrement
        curr_process->burst_time--;
      } else {
        printf("\t/\tDecrementing P%d", curr_process->__pid);
        curr_process->burst_time--;
        // printf("\t/\tEnqueuing P%d for remaining %d", curr_process->__pid,
        //        curr_process->burst_time);
        curr_inc_quantum = 0;
        // rr_queue_enqueue(&queue, curr_process);
        curr_process = NULL;
      }
    } else {
      curr_process = rr_queue_dequeue(&queue);
      if (curr_process != NULL) {
        goto handle_curr_process;
      } else {
        printf("\t/\tWaiting...");
      }
    }
    printf("\n");

    /*
    if (curr_process != NULL) {
      if (curr_inc_quantum == p->time_quantum) {
        curr_inc_quantum = 0;
        // This process still needs to be processed -- enqueue this item
        printf("-> Enqueuing P%d for remaining %d\n", curr_process->__pid,
               curr_process->burst_time);
        rr_queue_enqueue(&queue, curr_process);
      } else {
        curr_inc_quantum++;
        // Continue using the current process
        int subtracted_burst_time = curr_process->burst_time - 1;
        if (subtracted_burst_time <= 0) {
          // This process is now finished
          printf("-> Completed P%d\n", curr_process->__pid);
          curr_process->burst_time = 0;
          curr_process = NULL;
        } else {
          // Continue processing
          curr_process->burst_time = subtracted_burst_time;
        }
      }
    } else {
      // Dequeue next available process
      rr_input_data_t *next_process = rr_queue_dequeue(&queue);
      if (next_process != NULL) {
        printf("-> Dequeued P%d\n", next_process->__pid);
        curr_process = next_process;
      }
    } */

    rr_queue_print(&queue);
    printf("---------------------\n");

    // curr_inc_quantum++;
    // curr_inc_quantum = curr_inc_quantum % p->time_quantum;
    cycle++;
    sleep(2);
  }

  /*
  do {
    printf("-> Cycle: %d\n", cycle);
    printf("PID\tArrival\tBurst\n");
    for (int i = 0; i < NUM_RR_PROCESSES; i++) {
      rr_input_data_t *curr = &p->rr_input_data[i];
      printf("P%d\t%d\t%d", i + 1, curr->arrival_time, curr->burst_time);
      if (curr->arrival_time == cycle) {
        printf("\t-> ENQUEUE\n");
        rr_queue_enqueue(&queue, curr);
      } else {
        printf("\n");
      }
    }

    rr_queue_print(&queue);

    printf("-> Dequeueing...\n");
    can_dequeue_next_process = false;
    rr_input_data_t *data = rr_queue_dequeue(&queue);
    if (data != NULL) {
      printf("-> P%d dequeued\n", data->__pid);
      int subtracted_burst_time = data->burst_time - 1;
      if (subtracted_burst_time <= 0) {
        printf("-> Completed P%d\n", data->__pid);
        data->burst_time = 0;
      } else if (subtracted_burst_time > 0) {
        printf("-> Enqueing P%d again\n", data->__pid);
        data->burst_time = subtracted_burst_time;
        rr_queue_enqueue(&queue, data);
      }
    } else {
      printf("-> Nothing to process yet\n");
    }

    rr_queue_print(&queue);

    printf("---------------------\n");

    cycle++;
    sleep(1);
  } while (!done);
  */

  return NULL;
}

void *worker2(void *params) {
  // add your code here
  return NULL;
}
