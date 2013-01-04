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

#include <boost/chrono.hpp>
#include <boost/ref.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "ActiveMT.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

#define RUN_TIME          10 // s
#define QUEUE_SIZE         0 // 0 => unlimited

#define N_WORKERS         78
#define N_PRODUCERS       78

#define DELAY_ADD_THREAD  80 // ms
#define DELAY_RM_THREAD  100 // ms

#define DELAY_PRINT       75 // ms

#define DELAY_WORK       300 // ms

#define DELAY_PRODUCER   120 // ms

//-----------------------------------------------------------------------------

void work()
{
    boost::this_thread::sleep_for(boost::chrono::milliseconds(DELAY_WORK));
}

void produce( Active* ao )
{
    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist(1,10);

    while( true )
    {
        try
        {
            gen.seed(static_cast<unsigned int>(std::time(0)));
            int n = dist(gen);

            for( size_t i = 0; i < (size_t) n; ++i )
                if( !ao->send( &work ) )
                    std::cout << "!! failed to queue work\n" << std::flush;

            boost::this_thread::sleep_for(boost::chrono::milliseconds( n * DELAY_PRODUCER ));
        }
        catch ( boost::thread_interrupted& e )
        {
              break;
        }
    }
}

void print( Active* ao )
{
    while( true )
    {
        try
        {
            std::cout << "> threads [" << ao->tsize() << "] queue [" << ao->qsize() << "]\n" << std::flush;
            boost::this_thread::sleep_for(boost::chrono::milliseconds(DELAY_PRINT));
        }
        catch ( boost::thread_interrupted& e )
        {
              break;
        }
    }
}

void add_threads( Active* ao )
{
    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist(1,10);

    while( true )
    {
        try
        {
            gen.seed(static_cast<unsigned int>(std::time(0)));
            int n = dist(gen);

            Active::Message add = boost::bind( &Active::add_thread, ao );

            for( size_t i = 0; i < (size_t) n; ++i )
                ao->send(add);

            boost::this_thread::sleep_for(boost::chrono::milliseconds( n * DELAY_ADD_THREAD) );
        }
        catch ( boost::thread_interrupted& e )
        {
              break;
        }
    }
}

void remove_threads( Active* ao )
{
    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist(1,10);

    while( true )
    {
        try
        {
            gen.seed(static_cast<unsigned int>(std::time(0)));
            int n = dist(gen);

            Active::Message rm = boost::bind( &Active::remove_thread, ao );

            for( size_t i = 0; i < (size_t) n; ++i )
                ao->send(rm);

            boost::this_thread::sleep_for(boost::chrono::milliseconds( n * DELAY_RM_THREAD ));
        }
        catch ( boost::thread_interrupted& e )
        {
              break;
        }
    }
}

int main()
{
    std::cout << "> starting main" << std::endl;

    boost::shared_ptr<Active> ao = Active::create( N_WORKERS, QUEUE_SIZE );

    // start all auxiliary threads

    boost::thread p  ( print,          ao.get() );
    boost::thread at ( add_threads,    ao.get() );
    boost::thread rt ( remove_threads, ao.get() );

    // start all producer threads

    std::vector< Active::ThreadPtr > prod;
    for( size_t i = 0; i < N_PRODUCERS; ++i)
    {
        boost::shared_ptr<boost::thread> t( new boost::thread( produce, ao.get() ) );
        prod.push_back( t );
    }

    boost::this_thread::sleep_for( boost::chrono::seconds( RUN_TIME ) );

    // stop all producer threads

    for( size_t i = 0; i < prod.size(); ++i)
        prod[i]->interrupt();

    // stop all auxiliary threads

    at.interrupt();
    rt.interrupt();
    p.interrupt();

   // join all threads

    for( size_t i = 0; i < prod.size(); ++i)
        prod[i]->join();

    p.join();
    at.join();
    rt.join();

    std::cout << "> destroying ao" << std::endl;

    ao.reset();

    std::cout << "> ending main" << std::endl;
}
