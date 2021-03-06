/**
 * Active Objects using std c++11
 *
 * Multiple worker threads
 * Fixed size queue
 *
 **/

#define _GLIBCXX_USE_NANOSLEEP

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "message_queue.h"

//-----------------------------------------------------------------------------

#define HERE  std::cout << "--" << __LINE__ << std::endl;

//-----------------------------------------------------------------------------

#define QUEUE_SIZE  128 // 0 => unlimited

#define N_CONSUMERS 48
#define N_PRODUCERS 24

#define DELAY_PRINT 100 // ms
#define DELAY_WORK1 50  // ms x2
#define DELAY_WORK2 150 // ms

#define DELAY_PRODUCER 130 // ms

//-----------------------------------------------------------------------------

typedef std::function<void()> Message;

//-----------------------------------------------------------------------------

class Active {

private: // methods

    Active(const Active&) = delete;
    Active& operator=(const Active&) = delete;

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active();

    /// Flags the
    void finish(){ done_ = true; }
    void run();

private: // data

    bool                       done_;      ///< flag for finishing
    message_queue<Message>      mq_;        ///< message queue
    std::vector<std::thread>   threads_;   ///< multiple threads object

public: // methods

    /// Destructor
    /// Enqueue done message and wait for queue to drain
    virtual ~Active();

    /// Enqueue a message
    void send( Message msg );

    size_t qsize() { return mq_.size(); }

    /// Factory -- construction & thread start
    static std::unique_ptr<Active> create( size_t nb_threads = 1 );

};

//-----------------------------------------------------------------------------

Active::Active(): done_(false), mq_( QUEUE_SIZE )
{
}

Active::~Active()
{
  Message finish_msg = std::bind( &Active::finish, this );
  // enqueue finish message
  send(finish_msg);
  // wait for all processing in queue
  for( size_t i = 0; i < threads_.size(); ++i )
      threads_[i].join();
}

//-----------------------------------------------------------------------------

void Active::send( Message msg )
{
    mq_.wait_and_push(msg);
}

//-----------------------------------------------------------------------------

void Active::run()
{
  std::cout << "> starting run()\n" << std::flush;
  while (!done_)
  {
    Message f;
    mq_.wait_and_pop(f);
    f();
  }
  std::cout << "> ending run()\n" << std::flush;
}

//-----------------------------------------------------------------------------

std::unique_ptr<Active> Active::create( size_t nb_threads )
{
    std::unique_ptr<Active> pao( new Active() );
    for( size_t i = 0; i < nb_threads; ++i )
        pao->threads_.push_back(  std::thread(&Active::run, pao.get()) );
    return pao;
}

//-----------------------------------------------------------------------------

void work1()
{
//    std::cout << "-- work1() ...\n" << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_WORK1));
}

void work2()
{
//    std::cout << "-- work2() ...\n" << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_WORK2));
}

void produce( std::unique_ptr<Active>& ao )
{
    while( true )
    {
        ao->send( &work1 );
        ao->send( &work1 );
        ao->send( &work2 );

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_PRODUCER));
    }
}

void print( std::unique_ptr<Active>& ao )
{
    while( true )
    {
        std::cout << "> queue [" << ao->qsize() << "]\n" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_PRINT));
    }
}

int main()
{
    std::cout << "> starting main" << std::endl;

    std::unique_ptr<Active> ao = Active::create( N_CONSUMERS );

    std::vector<std::thread> prod;   ///< multiple producers
    for( size_t i = 0; i < N_PRODUCERS; ++i)
        prod.push_back( std::thread( produce, std::ref(ao) ) );

    std::thread p( print, std::ref(ao) );

    for( size_t i = 0; i < prod.size(); ++i)
        prod[i].join();

    p.join();

    std::cout << "> ending main" << std::endl;
}

