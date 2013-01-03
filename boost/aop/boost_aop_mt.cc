/**
 * Active Objects using std c++11
 *
 * Multiple worker threads
 * Fixed size queue
 *
 **/

#include <iostream>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/ref.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "shared_queue.h"
#include "ActiveMT.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

#define RUN_TIME     10 // s
#define QUEUE_SIZE  128 // 0 => unlimited

#define N_CONSUMERS 48
#define N_PRODUCERS 24

#define DELAY_ADD_THREAD 100 // ms
#define DELAY_RM_THREAD  200 // ms

#define DELAY_PRINT 80 // ms
#define DELAY_WORK1 50  // ms x2
#define DELAY_WORK2 150 // ms

#define DELAY_PRODUCER 20 // ms

//-----------------------------------------------------------------------------

void work1()
{
//    std::cout << "-- work1() ...\n" << std::flush;
    boost::this_thread::sleep_for(boost::chrono::milliseconds(DELAY_WORK1));
}

void work2()
{
//    std::cout << "-- work2() ...\n" << std::flush;
    boost::this_thread::sleep_for(boost::chrono::milliseconds(DELAY_WORK2));
}

void produce( boost::shared_ptr<Active>& ao )
{
    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist(1,10);

    while( true )
    {
        ao->send( &work1 );
        ao->send( &work1 );
        ao->send( &work2 );

        boost::this_thread::sleep_for(boost::chrono::milliseconds(dist(gen)*DELAY_PRODUCER));
    }
}

void print( boost::shared_ptr<Active>& ao )
{
    while( true )
    {
        std::cout << "> threads [" << ao->tsize() << "] queue [" << ao->qsize() << "]\n" << std::flush;
        boost::this_thread::sleep_for(boost::chrono::milliseconds(DELAY_PRINT));
    }
}

void add_threads( boost::shared_ptr<Active>& ao )
{
    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist(1,10);

    while( true )
    {
        Active::Message add = boost::bind( &Active::add_thread, ao );
        ao->send(add);
        boost::this_thread::sleep_for(boost::chrono::milliseconds( dist(gen)*DELAY_ADD_THREAD) );
    }
}

void remove_threads( boost::shared_ptr<Active>& ao )
{
    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist(1,10);

    while( true )
    {
        Active::Message rm = boost::bind( &Active::remove_thread, ao );
        ao->send(rm);
        ao->send(rm);
        boost::this_thread::sleep_for(boost::chrono::milliseconds( dist(gen)*DELAY_RM_THREAD ));
    }
}

int main()
{
    std::cout << "> starting main" << std::endl;

    boost::shared_ptr<Active> ao = Active::create( N_CONSUMERS, QUEUE_SIZE );

    // start all auxiliary threads

    boost::thread p  ( print,          boost::ref(ao) );
    boost::thread at ( add_threads,    boost::ref(ao) );
    boost::thread rt ( remove_threads, boost::ref(ao) );


    // start all producer threads

    std::vector< Active::ThreadPtr > prod;
    for( size_t i = 0; i < N_PRODUCERS; ++i)
    {
        boost::shared_ptr<boost::thread> t( new boost::thread( produce, boost::ref(ao) ) );
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

//    // join all threads

//    for( size_t i = 0; i < prod.size(); ++i)
//        prod[i]->join();

//    p.join();
//    at.join();
//    rt.join();

    std::cout << "> ending main" << std::endl;
}

