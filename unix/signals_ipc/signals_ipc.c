// Using kill for Communication
//
// http://www.gnu.org/s/hello/manual/libc/Kill-Example.html
//
// Here is a longer example showing how signals can be used for interprocess communication.
// This is what the SIGUSR1 and SIGUSR2 signals are provided for.
// Since these signals are fatal by default, the process that is supposed to receive them must trap
// them through signal or sigaction.
//
// In this example, a parent process forks a child process and then waits for the child to
// complete its initialization. The child process tells the parent when it is ready by
// sending it a SIGUSR1 signal, using the kill function.

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

/* When a SIGUSR1 signal arrives, set this variable.   */
volatile sig_atomic_t usr_interrupt = 0;

void synch_signal (int sig)
{
  usr_interrupt = 1;
}

/* The child process executes this function.  */
void child_function (void)
{
  /* Perform initialization.  */
  printf("child pid is %d\n", (int) getpid() );

  int i;
  for( i = 0; i < 1000; )
    ++i;

  /* Let parent know you're done.  */
  kill (getppid (), SIGUSR1);

  /* Continue with execution.  */
  printf("child is finishing %d\n", i);
  exit (0);
}

int main (void)
{
  printf("parent pid is %d\n", (int) getpid() );

  struct sigaction usr_action;
  sigset_t block_mask;
  pid_t child_id;

  /* Establish the signal handler.  */
  sigfillset (&block_mask);
  usr_action.sa_handler = synch_signal;
  usr_action.sa_mask = block_mask;
  usr_action.sa_flags = 0;
  sigaction (SIGUSR1, &usr_action, NULL);

  /* Create the child process.  */
  child_id = fork ();
  if (child_id == 0)
    child_function ();      /* Does not return.   */

  /* Busy wait for the child to send a signal.  */
  struct timespec ts; /* how much to wait */
  ts.tv_sec  = 0;
  ts.tv_nsec = 100;

  while (!usr_interrupt)
  {
    int err = nanosleep(&ts,NULL);
    if(err == -1)
      printf("interrupted\n");
    else
      printf(".\n");
  }

  /* Now continue execution.  */
  printf("parent is finishing\n");

  return 0;
}

// This example uses a busy wait, which is bad, because it wastes CPU cycles that other
// programs could otherwise use. It is better to ask the system to wait until the signal
// arrives. See the example in section "Waiting for a Signal".