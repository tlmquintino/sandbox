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

//template < typename M1, typename R1, typename R2 >
//void pipe_dispatch( M1 m, boost::function<R1(M1)> f, boost::shared_ptr< boost::promise<R1> > p, Active<R1,R2>& sink )
//{
//    p->set_value( f( m ) );
//    sink.send( p->get_future().get() );
//}

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

//    Active<int,void> printer( &print );

//    Active<int,int> tener( &times_ten, N_WORKERS, QUEUE_SIZE );
//    tener.dispatcher( boost::bind( &pipe_dispatch<int,int,void>, _1, _2, _3, boost::ref(printer) ) );

//    for( int i = 0; i < 200; ++i)
//        tener.send(i);

    std::cout << "> ending main" << std::endl;
}
