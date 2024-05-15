/*******************************************************************************
The assignment 3 for subject 48450 (RTOS) in University of Technology
Sydney(UTS) This is a template of Program_2.c template. Please complete the code
based on the assignment 3 requirement. Assignment 3

------------------------------Program_2.c template------------------------------
*******************************************************************************/

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Number of pagefaults in the program
int page_faults = 0;

// Function declaration
void signal_handler(int signal);

/**
 * Main routine for the program. In charge of setting up threads and the FIFO.
 *
 * @param argc Number of arguments passed to the program.
 * @param argv array of values passed to the program.
 * @return returns 0 upon completion.
 */
int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "USAGE: %s number_of_frames \n", argv[0]);
    return EXIT_FAILURE;
  }

  // Register Ctrl+C (SIGINT) signal
  signal(SIGINT, signal_handler);

  int i;
  // reference number
  int REFERENCES_STRING_LEN = 24;
  // Argument from the user on the frame size, such as 4 frames in the document
  int frameSize = atoi(argv[1]);
  // Frame where we will be storing the references. -1 is equivalent to an empty
  // value
  uint frame[REFERENCES_STRING_LEN];
  // Reference string from the assignment outline
  int referenceString[24] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3,
                             0, 3, 2, 1, 2, 0, 1, 7, 0, 1, 7, 5};
  // Next position to write a new value to.
  int nextWritePosition = 0;
  // Boolean value for whether there is a match or not.
  bool match = false;
  // Current value of the reference string.
  int currentValue;

  // Initialise the empty frame with -1 to simulate empty values.
  for (i = 0; i < frameSize; i++) {
    frame[i] = -1;
  }

  // Loop through the reference string values.
  for (i = 0; i < REFERENCES_STRING_LEN; i++) {
    // add your code here
  }

  // Sit here until the ctrl+c signal is given by the user.
  while (1) {
    sleep(1);
  }

  return 0;
}

/**
 * Performs the final print when the signal is received by the program.
 *
 * @param signal An integer values for the signal passed to the function.
 */
void signal_handler(int signal) {
  // add your code
  printf("\nTotal page faults: %d\n", page_faults);
  exit(0);
}
