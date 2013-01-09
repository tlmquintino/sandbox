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

//template < typename M >
//class Pipe : private boost::noncopyable {
//public: // methods
//    Pipe( ActiveT<M> source, ActiveT<M> sink );
//};


//-----------------------------------------------------------------------------

int times_ten( int i  )
{
   return i*10;
}

void print( int i )
{
    std::cout << i << std::endl;
}


template < typename AO >
class PipeDispatcher {

public: // methods

    typedef typename AO::result_type   pipe_output_t;
    typedef typename AO::message_type  pipe_input_t;

    typedef void result_type;
    typedef boost::shared_ptr< boost::promise<pipe_input_t> > promise_t;

    PipeDispatcher( AO& o ) : sink_(o) {}

    template< typename M >
    void operator() ( boost::function< pipe_input_t ( M ) > exec, M m, promise_t p )
    {
        p->set_value( exec( m ) );
        sink_.send( p->get_future().get() );
    }

private: // members

    AO& sink_;

};

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    typedef Active<int,void> IntSink;

    IntSink printer( &print );

    Active< int, int, PipeDispatcher<IntSink> > tener( &times_ten, printer, N_WORKERS, QUEUE_SIZE );

    for( int i = 0; i < 200; ++i)
        tener.send(i);

    std::cout << "> ending main" << std::endl;
}
