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

/** The name of the FIFO. */
#define FIFO_NAME "/tmp/rr-fifo"
/** The maximum string length of the output file name. */
#define OUTPUT_FILE_NAME_LEN 100

/* --- Global variables --- */

pthread_attr_t attr;

/* --- Structs --- */

/**
 * Input data for each process.
 */
typedef struct rr_process_t {
  /**
   * Unique identifier for this process.
   */
  int pid;
  /**
   * When does the process arrive (in milliseconds)?
   */
  int arrival_time;
  /**
   * CPU cycle count for this process to execute (in milliseconds).
   */
  int burst_time;
  /**
   * Number of CPU cycles this process has executed so far.
   */
  int exec_time;
  /**
   * Total duration this process has not been executing, from the moment it
   * spawned.
   */
  int wait_time;
  /**
   * The moment in which this process completed its execution.
   */
  int completion_time;
  /**
   * The moment in which this process became idle.
   *
   * There are two cases where this may happen:
   *
   *   1. The process has spawned but the scheduler is busy with another
   *      process. In this case, this process will move into the queue and wait
   *      for its turn.
   *
   *   2. The process has exceeded the time quantum while executing and must be
   *      put back in the queue. In this case, the process must wait for its
   *      turn.
   */
  int last_wait_start;
} rr_process_t;

/**
 * Thread parameters for the Round Robin (RR) scheduler.
 */
typedef struct thread_params_t {
  long int time_quantum;
  rr_process_t processes[NUM_RR_PROCESSES];
  char output_file[OUTPUT_FILE_NAME_LEN];
} thread_params_t;

/**
 * Represents the item type of the `rr_queue_t` linked list.
 *
 * Essentially, this is a linked list node with a reference to the next node in
 * the collection.
 */
typedef struct rr_queue_node_t {
  /**
   * A reference to a process in an array defined somewhere else.
   *
   * Must NOT be `NULL`, otherwise this is a logical error.
   */
  rr_process_t *process;
  /**
   * A reference to the next node.
   *
   * May be `NULL` if this node is the last one in the linked list.
   */
  struct rr_queue_node_t *next;
} rr_queue_node_t;

/**
 * Data collection type used to queue processes in the RR scheduler.
 *
 * Essentially, this is a linked list, where the first node represents the
 * oldest item added to the queue (i.e. the first to arrive), and the last node
 * represents the newest item added to the queue (i.e. the last to arrive).
 *
 * This collection type will constantly update as nodes are dequeued and
 * enqueued, gradually shifting all the nodes to the start of the queue until
 * there are no more nodes left to account for.
 */
typedef struct rr_queue_t {
  /**
   * A reference to the first node in the linked list. May be `NULL`.
   */
  rr_queue_node_t *first;
  /**
   * A reference to the last node in the linked list. May be `NULL`.
   */
  rr_queue_node_t *last;
} rr_queue_t;

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

/**
 * Initializes a process with the given PID, arrival time and burst time.
 */
void rr_process_init(rr_process_t *p, int pid, int at, int bt);

/**
 * Initializes the queue.
 */
void rr_queue_init(rr_queue_t *queue);

/**
 * Is the queue empty?
 */
int rr_queue_is_empty(rr_queue_t *queue);

/**
 * Adds a new process to the end of the queue.
 */
void rr_queue_enqueue(rr_queue_t *queue, rr_process_t *process);

/**
 * Takes out the next available process (the first one) from the queue.
 *
 * The process will be removed from the queue and the rest of the nodes will
 * "shift" to the start of the queue.
 *
 * If the queue is empty, this will return `NULL`.
 */
rr_process_t *rr_queue_dequeue(rr_queue_t *queue);

/**
 * Helper method to print out all the nodes currently in the queue.
 *
 * TODO: Only for debug purposes, this may be removed.
 */
void rr_queue_print(rr_queue_t *queue);

/* --- Main Code --- */

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

  // Initialize all processes for Round Robin scheduling simulation
  rr_process_t processes[NUM_RR_PROCESSES];
  rr_process_init(&processes[0], 1, 8, 10);
  rr_process_init(&processes[1], 2, 10, 3);
  rr_process_init(&processes[2], 3, 14, 7);
  rr_process_init(&processes[3], 4, 9, 5);
  rr_process_init(&processes[4], 5, 16, 4);
  rr_process_init(&processes[5], 6, 21, 6);
  rr_process_init(&processes[6], 7, 26, 2);

  for (int i = 0; i < NUM_RR_PROCESSES; i++) {
    params.processes[i] = processes[i];
  }

  // Create a FIFO (named pipe) with read/write permission
  int fifo_result = mkfifo(FIFO_NAME, 0777);
  if (fifo_result < 0) {
    perror("Failed to create FIFO");
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

  // Create second thread for `worker2`
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

  // Unlink the FIFO
  unlink(FIFO_NAME);

  return EXIT_SUCCESS;
}

void *worker1(void *params) {
  thread_params_t *p = params;

  int cycle = 0;
  int deadline = 0;
  int done = false;

  rr_queue_t queue;
  rr_process_t *curr_process = NULL;

  do {
#if DEBUG
    printf("-> CYCLE %d\n", cycle);
#endif
    // Check if any processes arrives at this cycle and add it to the queue
    done = true;
    for (int i = 0; i < NUM_RR_PROCESSES; i++) {
      rr_process_t *proc = &p->processes[i];
      int is_process_done = proc->exec_time == proc->burst_time;
      done = done && is_process_done;
      if (proc->arrival_time == cycle) {
#if DEBUG
        printf("-> ARRIVE(P%d, #%d)\n", proc->pid, cycle);
#endif
        proc->last_wait_start = cycle;
        rr_queue_enqueue(&queue, proc);
      }
    }

    if (curr_process == NULL) {
      // If no process is currently executing, attempt to dequeue the next
      // process in the queue
      curr_process = rr_queue_dequeue(&queue);
      if (curr_process != NULL) {
        // Bump the deadline to a maximum of current cycle + time quantum
        deadline = cycle + p->time_quantum;
        int wait_time = (cycle - curr_process->last_wait_start);
        int acc_wait_time = curr_process->wait_time + wait_time;
#if DEBUG
        printf("-> WT(P%d, %d + %d = %d)\n", curr_process->pid,
               curr_process->wait_time, wait_time, acc_wait_time);
#endif
        curr_process->wait_time = acc_wait_time;
      }
    }

    if (curr_process != NULL) {
#if DEBUG
      printf("-> DEADLINE(%d)\n", deadline);
      printf("[ P%d\t: %d\t: %d\t: %d\t: %d\t: %d\t: %d\t]\n",
             curr_process->pid, curr_process->arrival_time,
             curr_process->burst_time, curr_process->exec_time,
             curr_process->last_wait_start, curr_process->wait_time,
             curr_process->completion_time);
#endif

      // Simulate an execution cycle for this process
      curr_process->exec_time++;
#if DEBUG
      printf("-> EXEC(P%d, %d)\n", curr_process->pid, curr_process->exec_time);
#endif

      // Check if the process should be retired or enqueued
      if (curr_process->exec_time == curr_process->burst_time) {
        // This process is now completed, it can now be retired
#if DEBUG
        printf("-> RETIRE(P%d, %d)\n", curr_process->pid,
               curr_process->exec_time);
#endif
        curr_process->completion_time = cycle + 1;
#if DEBUG
        printf("-> COMPLETE(P%d, %d)\n", curr_process->pid,
               curr_process->completion_time);
#endif
        curr_process = NULL;
      } else if (cycle + 1 == deadline) {
#if DEBUG
        printf("-> ENQUEUE(P%d)\n", curr_process->pid);
#endif
        // This is the last cycle to execute this process, it should be enqueued
        // to be completed at a later time
        rr_queue_enqueue(&queue, curr_process);
        curr_process->last_wait_start = cycle + 1;
        curr_process = NULL;
      }
    }

#if DEBUG
    rr_queue_print(&queue);
    printf("=================================\n");
#endif
    cycle++;
  } while (!done);

  float avg_wait_time = 0.0;
  float avg_turn_around_time = 0.0;

#if DEBUG
  printf("Results:\n\n");
  printf("\tProcess\tArrive\tBurst\tWT\tCT\tTAT\n");
#endif
  for (int i = 0; i < NUM_RR_PROCESSES; i++) {
    rr_process_t *proc = &p->processes[i];
    int turn_around_time = proc->completion_time - proc->arrival_time;
    avg_wait_time += proc->wait_time;
    avg_turn_around_time += turn_around_time;
#if DEBUG
    printf("\tP%d\t%d\t%d\t%d\t%d\t%d\n", proc->pid, proc->arrival_time,
           proc->burst_time, proc->wait_time, proc->completion_time,
           turn_around_time);
#endif
  }

  avg_wait_time /= NUM_RR_PROCESSES;
  avg_turn_around_time /= NUM_RR_PROCESSES;
#if DEBUG
  printf("\n");
  printf("Average Wait Time: %fms\n", avg_wait_time);
  printf("Average Turn Around Time: %fms\n", avg_turn_around_time);
#endif

  int fifo_fd = open(FIFO_NAME, O_WRONLY);
  if (fifo_fd < 0) {
    perror("Failed to open FIFO with write access");
    exit(EXIT_FAILURE);
  }

  write(fifo_fd, &avg_wait_time, sizeof(avg_wait_time));
  write(fifo_fd, &avg_turn_around_time, sizeof(avg_turn_around_time));

  // Close the FIFO
  close(fifo_fd);
  remove(FIFO_NAME);

  return NULL;
}

void *worker2(void *params) {
  thread_params_t *p = params;

  float fifo_avg_wait_time = 0.0;
  float fifo_avg_turn_around_time = 0.0;

  int fifo_fd = open(FIFO_NAME, O_RDONLY);

  if (fifo_fd < 0) {
    perror("Failed to open FIFO with read access");
    exit(EXIT_FAILURE);
  }

  read(fifo_fd, &fifo_avg_wait_time, sizeof(int));
  read(fifo_fd, &fifo_avg_turn_around_time, sizeof(int));

#if DEBUG
  printf("FIFO: Average Wait Time: %fms\n", fifo_avg_wait_time);
  printf("FIFO: Average Turn Around Time: %fms\n", fifo_avg_turn_around_time);
#endif

  FILE *output_file;
  if ((output_file = fopen(p->output_file, "w")) == NULL) {
    perror("Failed to create/open output file");
    exit(EXIT_FAILURE);
  }

  fprintf(output_file, "Average Wait Time: %fms\n", fifo_avg_wait_time);
  fprintf(output_file, "Average Turn Around Time: %fms\n",
          fifo_avg_turn_around_time);

  fclose(output_file);
  close(fifo_fd);

  return NULL;
}

/* --- RR Queue Methods --- */

void rr_process_init(rr_process_t *p, int pid, int at, int bt) {
  p->pid = pid;
  p->arrival_time = at;
  p->burst_time = bt;
  p->exec_time = 0;
  p->wait_time = 0;
  p->completion_time = 0;
  p->last_wait_start = 0;
}

void rr_queue_init(rr_queue_t *queue) {
  queue->first = NULL;
  queue->last = NULL;
}

int rr_queue_is_empty(rr_queue_t *queue) {
  return queue->first == NULL && queue->last == NULL;
}

void rr_queue_enqueue(rr_queue_t *queue, rr_process_t *process) {
  rr_queue_node_t *new_node =
      (rr_queue_node_t *)malloc(sizeof(rr_queue_node_t));
  new_node->process = process;
  new_node->next = NULL;

  if (rr_queue_is_empty(queue)) {
    queue->last = new_node;
    queue->first = queue->last;
  } else {
    queue->last->next = new_node;
    queue->last = new_node;
  }
}

rr_process_t *rr_queue_dequeue(rr_queue_t *queue) {
  if (rr_queue_is_empty(queue)) {
    return NULL;
  }

  rr_queue_node_t *node_to_remove = queue->first;
  rr_process_t *dequeued_process = node_to_remove->process;

  if (queue->first == queue->last) {
    // If there is only one node left, unset first and last nodes
    queue->last = NULL;
    queue->first = NULL;
  } else {
    // Otherwise, set the first node to the second node (i.e. shift the nodes)
    queue->first = queue->first->next;
  }

  free(node_to_remove);
  return dequeued_process;
}

void rr_queue_print(rr_queue_t *queue) {
  rr_queue_node_t *peek_node = queue->first;
  printf("| Proc\t| Arriv\t| Burst\t| Exec\t| LWS\t| WT\t| CT\t|\n");
  while (peek_node != NULL) {
    rr_process_t *proc = peek_node->process;
    printf("| P%d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t|\n", proc->pid,
           proc->arrival_time, proc->burst_time, proc->exec_time,
           proc->last_wait_start, proc->wait_time, proc->completion_time);
    peek_node = peek_node->next;
  }
}
