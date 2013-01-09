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

void print_i( int i )
{
    std::cout << i << std::endl;
}

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    Active<int,int> tener( &times_10 );

    Active<int,int>::promise_ptr p1 = tener.send( 20 );

    assert( p1->get_future().get() == 200 );

    Active<int,void> printer( &print_i );

    Active<int,void>::promise_ptr p2 = printer.send( 20 );

    std::cout << "> ending main" << std::endl;
}


