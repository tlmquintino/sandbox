#include <iostream>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <string>


int main(int argc, char** argv)
{
     std::cout << "size of long    [" << sizeof(long) << "]" << std::endl;
     std::cout << "size of off_t   [" << sizeof(off_t) << "]" << std::endl;

#ifdef off64_t
     std::cout << "size of off64_t [" << sizeof(off64_t) << "]" << std::endl;
#endif

    std::string filename;
    if(argc > 1)
      filename = argv[1];
    else
      filename = argv[0]; // open itself if no file passed

    FILE* f = fopen(filename.c_str(), "r");
    if(!f) perror("fopen"), exit(EXIT_FAILURE);

#if USE_FSEEKO
    off_t size;
    fseeko (f, 0, SEEK_END);
    size = ftello (f);
#else
    long size;
    fseek (f, 0, SEEK_END);
    size = ftell (f);
#endif

    std::cout << filename << " is " << size << " [bytes]" << std::endl;

    return 0;
}
