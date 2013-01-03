/**
 * Active Objects using boost and c++11
 *
 * Single worker thread
 *
 **/

#include <iostream>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/ref.hpp>

#include "ActiveST.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

void foo()
{
    std::cout << "[1] foo() ...\n";
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
}

void bar()
{
    std::cout << "[2] bar() ...\n";
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
}

int main()
{
    std::cout << "> starting main" << std::endl;

    boost::shared_ptr<Active> ao = Active::create();

    for( size_t i = 0; i < 5; ++i )
    {
        ao->send( &foo );
        ao->send( &bar );
    }

    std::cout << "> ending main" << std::endl;
}

