#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILEPATH "mmapped.bin"
#define NUMINTS  (1000)
#define FILESIZE (NUMINTS * sizeof(int))

int main(int argc, char *argv[])
{
    int i;
    int fd;
    int result;
    int *map;  /* mmapped array of int's */

    // Note "O_WRONLY" mode is not sufficient when mmaping
    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1)  perror("error opening file for writing"), exit(EXIT_FAILURE);

    // stretch the file size to the size of the (mmapped) array of ints
    if( ::lseek(fd, FILESIZE-1, SEEK_SET) < 0 ) perror("error calling lseek() to 'stretch' the file"), exit(1);

    // write and empty string at end to size the fiel correctly
    if( write(fd, "", 1) != 1) close(fd), perror("error writing last byte of the file"), exit(EXIT_FAILURE);

    // file is ready to be maped
    map = (int*) mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) close(fd), perror("error mmapping the file"), exit(EXIT_FAILURE);

    // write to the file as if it were memory
    for (i = 1; i <=NUMINTS; ++i)
      map[i] = 2 * i;

    // free the mmapped memory
    if (munmap(map, FILESIZE) == -1) perror("error un-mmapping the file"), exit(EXIT_FAILURE);

    // clsoe the file
    if( close(fd) == -1 ) perror("error closing the file"), exit(EXIT_FAILURE);
    return 0;
}