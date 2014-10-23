#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>

using namespace std;

#define DBGX(x) std::cout << "@line: " << __LINE__ << " " << #x << ":" << x << std::endl;
#define DBG     std::cout << "@line: " << __LINE__ << std::endl;

int main(int argc, char *argv[])
{

	if( argc < 2 ) throw std::runtime_error("must give a path");

	std::string path ( argv[1] );

  	/* initialize random seed: */
	srand( time(NULL) );

	pid_t pid = ::getpid();

    char host[1024];
    ::gethostname( host, sizeof(host)-1 );
	std::string hostname(host);

    char buffer[1024];

    // keep appending to the file with random time intervals

    for( size_t i = 0; i < 1000; ++i )
    {
    	DBGX(i);

	    // create message

		time_t rawtime;

  		::time( &rawtime );

	    std::ostringstream os;
	    os  << "host:"  << hostname << " "
	    	<< "pid:"   << pid << " "
	    	<< "entry:" << i << " "
	    	<< "time:"  << ::ctime(&rawtime);

	    std::string ss = os.str();

		// (re)open file

	    int fd = ::open( path.c_str(), O_APPEND|O_CREAT|O_WRONLY, 0644 );
	    if( fd < 0 ) perror("open"), printf("failed to open %s", path.c_str()), exit(1);

	    // append to the file 
	    if( ::write(fd, ss.c_str(), ss.size() ) < 0 )
	    	perror("write"), exit(1);

	    // close file

	    if( ::close(fd) < 0 )
	    	perror("close"), exit(1);

	    /* generate secret number between 1 and 10: */
  		unsigned int usecs = (rand() % 10 + 1) * 100000;
        ::usleep(usecs);

    }
}

