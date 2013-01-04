/**
 * Active Objects using std c++11
 *
 * Multiple worker threads
 * Fixed size queue
 *
 **/

#include <iostream>
#include <memory>

#include "ActiveT.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

int times_10( int i )
{
    return i * 10;
}

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    Active<int,int> tener( &times_10 );

    Active<int,int>::promise_ptr pi = tener.send( 20 );

    assert( pi->get_future().get() == 200 );

    std::cout << "> ending main" << std::endl;
}


