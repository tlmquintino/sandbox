#include <cassert>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include <limits>
#include <cmath>

#include <sys/mman.h>

using namespace std;

int main(int argc, char *argv[])
{
   int pagesize;
   errno = 0;
   if ((pagesize = sysconf(_SC_PAGE_SIZE)) == -1)
      if (errno == 0)
         printf("PAGE_SIZE not supported by this implementation.n");
      else
         perror("sysconf error.");
   else
      printf("PAGE_SIZE = %d\n", pagesize);

printf("1.\n");

    printf( "sizeof void*     %ld\n", sizeof(void*) );

printf("2.\n");

    printf( "sizeof short     %ld\n", sizeof(short) );
    printf( "sizeof int       %ld\n", sizeof(int) );
    printf( "sizeof long int  %ld\n", sizeof(long int) );
    printf( "sizeof size_t    %ld\n", sizeof(size_t) );
    printf( "sizeof off_t     %ld\n", sizeof(off_t) );

printf("3.\n");

    printf( "max off_t can index %.1f EB (1 ExaByte = 1x10^18 Bytes )\n", std::pow(2., (int) sizeof(off_t)*8) / 1E18  );

printf("4.\n");

#define LIMIT_2GB 2147483648

    // compute the 2GB limit and its number of pages
    size_t nbpages_2gb = LIMIT_2GB / pagesize;

    assert( nbpages_2gb * pagesize == LIMIT_2GB );

    printf( "The 2GB limit is actually %ld pages\n", nbpages_2gb );

    size_t size_2g = ( nbpages_2gb+1 ) * pagesize;

printf("5.\n");

    // lock a file after the 2Gb limit
    int fd = ::open("/dev/zero", O_RDWR );
    if( fd < 0 ) printf("failed to open /dev/zero"), exit(1);


    // map the memory
    void* madd = mmap(0, size_2g, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if( madd == MAP_FAILED) perror("mmap"), exit(1);

    struct flock fl;

    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = LIMIT_2GB;
    fl.l_len    = 0;
    fl.l_pid    = getpid();

    // lock the region beyond the 2GB limit
    fcntl(fd, F_SETLKW, &fl);

    // unlock the region
    fl.l_type   = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);

        return 0;
}

