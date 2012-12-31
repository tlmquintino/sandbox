/**
 * Active Objects using boost and c++11
 *
 * @todo
 *      * add kill message to remove one thread
 *      * return futures
 *      * make queue size fixed -- send may block
 *      * what to do to messages queued after the 'done' message?
 *
 **/


#include <iostream>
#include <memory>

#include <boost/thread.hpp>

#include "shared_queue.h"

#define HERE  std::cout << "--" << __LINE__ << std::endl;

typedef std::function<void()> Message;

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

    shared_queue<Message>      mq_;        ///< message queue
    bool                       done_;      ///< flag for finishing
    std::vector<boost::thread> threads_;   ///< multiple threads object

public: // methods

    /// Destructor
    /// Enqueue done message and wait for queue to drain
    virtual ~Active();

    /// Enqueue a message
    void send( Message msg );

    /// Factory -- construction & thread start
    static std::unique_ptr<Active> create( size_t nb_threads = 1 );

};

//-----------------------------------------------------------------------------

Active::Active(): done_(false)
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
    mq_.push(msg);
}

//-----------------------------------------------------------------------------

void Active::run()
{
  std::cout << "> starting run()" << std::endl;
  while (!done_)
  {
    Message f;

    // timed wait so it gives chance to exit on done
    if( mq_.wait_for_and_pop(f,std::chrono::milliseconds(100)) )
        f();
    else
        std::cout << "> timeout waiting for work ..." << std::endl;
  }
  std::cout << "> ending run()" << std::endl;
}

//-----------------------------------------------------------------------------

std::unique_ptr<Active> Active::create( size_t nb_threads )
{
    std::unique_ptr<Active> pao( new Active() );
    for( size_t i = 0; i < nb_threads; ++i )
        pao->threads_.push_back(  boost::thread(&Active::run, pao.get()) );
    return pao;
}

//-----------------------------------------------------------------------------

void foo()
{
    std::cout << "[1] foo() ...\n";
    ::sleep(1);
}

void bar()
{
    std::cout << "[2] bar() ...\n";
    ::sleep(1);
}

int main()
{
    std::cout << "> starting main" << std::endl;

    std::unique_ptr<Active> ao = Active::create( 7 );

    for( size_t i = 0; i < 20; ++i )
    {
        ao->send( &foo );
        ao->send( &foo );
        ao->send( &bar );
    }

    std::cout << "> ending main" << std::endl;
}

