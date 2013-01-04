/**
 * Active Objects using std c++11
 *
 * Multiple worker threads
 * Fixed size queue
 *
 **/

#define BOOST_THREAD_VERSION 3

#include <iostream>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/ref.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "ActiveT.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

#define N_WORKERS    10

#define QUEUE_SIZE  128

//-----------------------------------------------------------------------------

typedef boost::function< void( int ) > dispatch_t;

void times_ten( int i )
{
    std::cout << "i [" << i << "]" << std::endl;
}

int main()
{
    std::cout << "> starting main" << std::endl;

    boost::shared_ptr< Active<int> > ao( new Active<int>( &times_ten, N_WORKERS, QUEUE_SIZE ) );

    for( int i = 0; i < 20; ++i)
        ao->send(i);

    ao.reset();

    std::cout << "> ending main" << std::endl;
}
