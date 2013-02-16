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
#include <boost/lexical_cast.hpp>

#include "Active.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

#define N_WORKERS    10

#define QUEUE_SIZE  128

//-----------------------------------------------------------------------------

std::string to_str( int i  )
{
    std::ostringstream oss;
    oss << i;
    return oss.str();
}

double to_dbl( std::string s )
{
    std::cout << "[" << s << "]" << std::endl;
    return boost::lexical_cast<double>(s);

}

void print( double d )
{
    std::cout << "Double [" << d << "]" << std::endl;
}

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    typedef Active<int,std::string>    IntToStr;
    typedef Active<std::string,double> StrToDbl;
    typedef Active<double,void>        DblToVoid;

    typename IntToStr::Ptr toStr  = IntToStr::create( &to_str, N_WORKERS, QUEUE_SIZE  );
    typename StrToDbl::Ptr toDbl  = StrToDbl::create( &to_dbl );
    typename DblToVoid::Ptr pDbl  = DblToVoid::create( &print );


    IActive< double >::Ptr p = toStr | toDbl | pDbl;

//    p->send( 10 );

//    Pipe2<IntToStr,StrToDbl> p (toStr , toDbl);

//    for( int i = 0; i < 200000; ++i)
//        p.send(i);

//    Pipe<int,double> p = toStr | toDbl;

//    boost::promise<double> d = p.send( 20 );
//    boost::future<double> df = d.get_future();

//    df.wait();

//    std::cout << "> value " << df.get() << std::endl;

    std::cout << "> ending main" << std::endl;
}
