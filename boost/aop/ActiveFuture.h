#ifndef Active_h
#define Active_h

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "shared_queue.h"

//-----------------------------------------------------------------------------

class Active : private boost::noncopyable {

public: // types

    typedef boost::function< void() >          Message;
    typedef boost::shared_ptr< boost::thread > ThreadPtr;

private: // methods

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active();

    /// Flags the Active object to finish
    void finish(){ done_ = true; }

    /// Keeps executing the messages that are queued
    void run();

private: // data

    typedef std::map< boost::thread::id, ThreadPtr > ThreadPool;

    bool                      done_;      ///< flag for finishing
    shared_queue<Message>     mq_;        ///< message queue

    mutable boost::mutex      m_;         ///< mutex for locking changes in the Active object itself

    ThreadPool                threads_;   ///< multiple threads object

public: // methods

    /// Destructor
    /// Enqueue done message and wait for queue to drain
    virtual ~Active();

    /// Enqueue a message
    bool send( Message msg );

    /// Adds one more thread
    void add_thread();

    /// Removes the thread that executes the Message
    void remove_thread();

    /// @returns the nb worker threads
    size_t tsize()
    {
        boost::lock_guard<boost::mutex> lock(m_);
        return threads_.size();
    }

    /// @returns the current queue size
    size_t qsize() { return mq_.size(); }

    /// sets the maximum queue size
    void qsize( const size_t& s ) { mq_.max_size(s); }

    /// Factory -- construction & thread start
    static boost::shared_ptr<Active> create( size_t nb_threads = 1, size_t qsize = 0 );

};

//-----------------------------------------------------------------------------

Active::Active(): done_(false)
{
}

Active::~Active()
{
}

//-----------------------------------------------------------------------------

bool Active::send( Message msg )
{
    return mq_.wait_and_push(msg);
}

void Active::add_thread()
{
    boost::lock_guard<boost::mutex> lock(m_);

    ThreadPtr p( new boost::thread(&Active::run, this) );

    threads_[ p->get_id() ] = p;
}

void Active::remove_thread()
{
    boost::lock_guard<boost::mutex> lock(m_);

    if( threads_.size() == 1 ) // never remove last thread
        return;

    boost::thread::id tid = boost::this_thread::get_id();
    ThreadPool::iterator itr = threads_.find( tid );

    if( itr != threads_.end() )
    {
        ThreadPtr p = itr->second;
        assert( p->get_id() == tid );
        threads_.erase( itr );
        p->interrupt();
        boost::this_thread::interruption_point();
    }
}

//-----------------------------------------------------------------------------

void Active::run()
{
//  std::cout << "> starting run()\n" << std::flush;
  while (!done_)
  {
    try
    {
        Message f;
        mq_.wait_and_pop(f);
        f();
    }
    catch ( boost::thread_interrupted& e )
    {
//          std::cout << "> interrupt thread ...\n" << std::flush;
          break;
    }
  }
//  std::cout << "> ending run()\n" << std::flush;
}

//-----------------------------------------------------------------------------

boost::shared_ptr<Active> Active::create( size_t nb_threads, size_t qsize )
{
    boost::shared_ptr<Active> pao( new Active() );

    // attach one thread

    ThreadPtr p( new boost::thread(&Active::run, pao.get()) );
    pao->threads_[ p->get_id() ] = p;

    for( size_t i = 1; i < nb_threads; ++i )
    {
        Message add = boost::bind( &Active::add_thread, pao.get() );
        pao->send(add);
    }

    pao->qsize( qsize );

    return pao;
}

#endif

