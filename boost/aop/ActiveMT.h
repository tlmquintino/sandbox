#ifndef Active_h
#define Active_h

#define BOOST_THREAD_VERSION 3

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

#if 0
    struct ThreadPool : public std::map< boost::thread::id, ThreadPtr >
    {
        ~ThreadPool()
        {
            // wait for all threads still processing queue messages to exit normally
            for( ThreadPool::iterator i = begin(); i != end(); ++i )
                if( i->second->joinable() )
                    i->second->join();
        }
    };
#else
    typedef std::map< boost::thread::id, ThreadPtr > ThreadPool;
#endif

private: // methods

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active();

    /// Joins threads no longer used
    void join_threads();

    /// Keeps executing the messages that are queued
    void run();

private: // data

    bool                      done_;          ///< flag for finishing
    mutable boost::mutex      m_;             ///< mutex for locking changes in the Active object itself

    ThreadPool                threads_;       ///< multiple threads object
    ThreadPool                intr_threads_;  ///< list of threads that were interrupted and need to be joined

    shared_queue<Message>     mq_;            ///< message queue

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
    size_t tsize();

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

//-----------------------------------------------------------------------------

Active::~Active()
{
    std::cout << "> waiting for the remaining tasks...\n" << std::flush;

    mq_.drain_and_close();
    done_ = true;

    //    std::cout << "> waiting for the remaining tasks... ( mq_.size() = " << mq_.size() << " )\n" << std::flush;

    boost::lock_guard<boost::mutex> lock(m_);

    // wait for all threads still processing queue messages to exit normally
    for( ThreadPool::iterator i = threads_.begin(); i != threads_.end(); ++i )
        if( i->second->joinable() )
            i->second->join();

    // wait for all threads still processing queue messages to exit normally
    for( ThreadPool::iterator i = intr_threads_.begin(); i != intr_threads_.end(); ++i )
        if( i->second->joinable() )
            i->second->join();
}

//-----------------------------------------------------------------------------

bool Active::send( Message msg )
{
    return mq_.wait_and_push(msg);
}

//-----------------------------------------------------------------------------

void Active::add_thread()
{
    if( mq_.is_open() ) //< only add if queue is open, else don't bother
    {
        boost::lock_guard<boost::mutex> lock(m_);

        ThreadPtr p( new boost::thread(&Active::run, this) );

        threads_[ p->get_id() ] = p;
    }
}

//-----------------------------------------------------------------------------

void Active::join_threads()
{
    boost::lock_guard<boost::mutex> lock(m_);

    for( ThreadPool::iterator i = intr_threads_.begin(); i != intr_threads_.end(); ++i )
        if( i->second->joinable() )
            i->second->join();

    intr_threads_.clear();
}

//-----------------------------------------------------------------------------

void Active::remove_thread()
{
    boost::lock_guard<boost::mutex> lock(m_);

    if( threads_.size() == 1 ) // never remove last thread
        return;

    if( ! mq_.is_open() ) // if queue is already closed don't bother
        return;

    boost::thread::id tid = boost::this_thread::get_id();
    ThreadPool::iterator itr = threads_.find( tid );

    if( itr != threads_.end() )
    {
        // erase the thread from pool

        ThreadPtr p = itr->second;
        assert( p->get_id() == tid );
        threads_.erase( itr );

        // add thread to join list
        intr_threads_[tid] = p;

        // try to queue joining threads, but may fail if queue !open
        // in which case we don't care since destuctor will join
        send( Message( boost::bind( &Active::join_threads, this ) ) );

        throw boost::thread_interrupted();
    }
}

//-----------------------------------------------------------------------------

size_t Active::tsize()
{
    boost::lock_guard<boost::mutex> lock(m_);
    return threads_.size();
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
            if(mq_.try_and_pop(f))
                f();
            else
                boost::this_thread::yield();
        }
        catch ( boost::thread_interrupted& e )
        {
              break; //< finish this thread, will be joined in intr_threads_
        }
    }
//  std::cout << "> ending run()" << std::flush;
}

//-----------------------------------------------------------------------------

boost::shared_ptr<Active> Active::create( size_t nb_threads, size_t qsize )
{
    boost::shared_ptr<Active> pao( new Active() );

    // attach one thread

    try
    {
        ThreadPtr p( new boost::thread(&Active::run, pao.get()) );
        pao->threads_[ p->get_id() ] = p;
    }
    catch(...)
    {
        pao.reset();
        return pao;
    }

    for( size_t i = 1; i < nb_threads; ++i )
    {
        Message add = boost::bind( &Active::add_thread, pao.get() );
        pao->send(add);
    }
    pao->qsize( qsize );

    return pao;
}

#endif

