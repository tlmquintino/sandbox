/**
 * Active Objects using std c++11
 *
 * Multiple worker threads
 * Fixed size queue
 *
 **/

#include <iostream>

#define BOOST_THREAD_VERSION 3

#include <boost/thread/future.hpp>

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    boost::promise<int> pi;
    boost::future<int> fi;
    fi=pi.get_future();

    pi.set_value(42);

    assert(fi.is_ready());
    assert(fi.has_value());
    assert(!fi.has_exception());
    assert(fi.get_state()==boost::future_state::ready);
    assert(fi.get()==42);

    std::cout << fi.get() << std::endl;


    std::cout << "> ending main" << std::endl;
}


