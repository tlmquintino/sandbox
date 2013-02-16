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

#include <boost/noncopyable.hpp>

#include "ActiveT.h"
#include "Pipe1.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

#define N_WORKERS    10

#define QUEUE_SIZE  128

//-----------------------------------------------------------------------------

int times_ten( int i  )
{
   return i*10;
}

void print( int i )
{
    std::cout << i << std::endl;
}

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    typedef Active<int,int>   IntToInt;
    typedef Active<int,void>  IntToVoid;

    typedef Pipe1<int,int,void> PipeIntToVoid;

    IntToInt   tener( &times_ten, N_WORKERS, QUEUE_SIZE );
    IntToVoid  printer( &print );

    boost::shared_ptr< PipeIntToVoid > p ( new PipeIntToVoid(tener,printer) );

    for( int i = 0; i < 2000; ++i)
        p->send(i);

    p.reset();

    std::cout << "> ending main" << std::endl;
}
