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

template < typename T1, typename T2 >
class Pipe : private boost::noncopyable {

public: // types

    typedef typename T1::message_type  message_type;
    typedef typename T2::result_type   result_type;

public: // methods

    Pipe( T1& source, T2& sink) :
        source_(source),
        sink_(sink)
    {
        source_.dispatcher( make_pipe_to( sink_ ) );
    }

    void send( message_type msg )
    {
        source_.send(msg);
    }

private: // members

    T1& source_;
    T2& sink_;

};

template < typename T1, typename T2 >
Pipe<T1,T2> make_pipe( T1& source, T2& sink )
{
    return Pipe<T1,T2>(source,sink);
}

//-----------------------------------------------------------------------------

/// @note Pipe solution between Active objects
///
///       Involves changing the dispatcher in each upstream AOP
///       Pipes are typed source_type to sink_type
///       Pipes don't return their result

///       Pipes aren't objects themselves -- SOLVED

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    typedef Active<int,void> SinkInt;
    typedef Active<int,int>  ProcessInt;

    SinkInt printer( &print );
    ProcessInt tener( &times_ten, N_WORKERS, QUEUE_SIZE );

    Pipe<ProcessInt,SinkInt> p (tener,printer);

    for( int i = 0; i < 200000; ++i)
        p.send(i);

    std::cout << "> ending main" << std::endl;
}
