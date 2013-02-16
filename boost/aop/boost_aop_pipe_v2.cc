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

#include "ActiveT.h"
//#include "Pipe2.h"

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

//-----------------------------------------------------------------------------

int main()
{
    std::cout << "> starting main" << std::endl;

    typedef Active<int,int>            IntToInt;
    typedef Active<int,std::string>    IntToStr;
    typedef Active<std::string,double> StrToDbl;

    IntToStr toStr( &to_str, N_WORKERS, QUEUE_SIZE  );
    StrToDbl toDbl( &to_dbl );

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
