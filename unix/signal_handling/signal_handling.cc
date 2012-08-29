// start it up in one console, and then use the kill -USR1 in another
// console to kill it.
// this program redirects the SIGINT and counts its occurences
// and can get out of the infinit loop by receiving a SIGUSR1

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

volatile sig_atomic_t got_sigusr1;
volatile sig_atomic_t got_sigint;

void sigusr1_handler(int sig)
{
    got_sigusr1 = 1;
}

void sigint_handler(int sig) // this will avoid SIGINT interrupting the process
{
    got_sigint = 1;
    write(0, "SIGINT!\n",8);
}

int main(void)
{
    got_sigusr1 = 0;
    got_sigint = 0;

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = sigusr1_handler;
    sa_usr1.sa_flags = 0;
    sigemptyset(&sa_usr1.sa_mask);

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) perror("sigaction"), exit(EXIT_FAILURE);

    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler;
    sa_int.sa_flags = SA_RESTART;
    sigemptyset(&sa_int.sa_mask);

    if (sigaction(SIGINT, &sa_int, NULL) == -1) perror("sigaction"), exit(EXIT_FAILURE);

    unsigned int nbsigints = 0;
    while (!got_sigusr1)
    {
        printf("PID %d: working hard...\n", getpid());
        sleep(1);
        if(got_sigint)
        {
          ++nbsigints;
          got_sigint = 0;
        }
    }

    printf("got out via SIGUSR1!\n");
    printf("received %d sigints\n", nbsigints);

    return 0;
}
