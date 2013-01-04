#ifndef Active_h
#define Active_h

#define BOOST_THREAD_VERSION 3

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "shared_queue.h"

//-----------------------------------------------------------------------------

template < typename M >
class Active : private boost::noncopyable {

public: // types

    typedef boost::function<void(M)> Dispatcher;

private: // types

    typedef boost::shared_ptr< boost::thread > ThreadPtr;
    typedef std::map< boost::thread::id, ThreadPtr > ThreadPool;


public: // methods

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active( Dispatcher d, size_t nb_threads = 1, size_t qsize = 0 );

    /// Destructor
    /// Enqueue done message and wait for queue to drain
    virtual ~Active();

    /// Enqueue a message
    bool send( M msg );

    /// @returns the nb worker threads
    size_t tsize();

    /// @returns the current queue size
    size_t qsize() { return mq_.size(); }

    /// sets the maximum queue size
    void qsize( const size_t& s ) { mq_.max_size(s); }

private: // methods

    void run();

private: // data

    bool                      done_;          ///< flag for finishing
    mutable boost::mutex      m_;             ///< mutex for locking changes in the Active object itself

    Dispatcher                dispatch_;      ///< function to handle each message

    ThreadPool                threads_;       ///< multiple threads object

    shared_queue<M>           mq_;            ///< message queue

};

//-----------------------------------------------------------------------------

template < typename M >
Active<M>::Active( Dispatcher d, size_t nb_threads, size_t qsize ) :
    done_(false),
    dispatch_(d),
    mq_(qsize)
{
    try
    {
        for( size_t i = 0; i < nb_threads; ++i )
        {
            ThreadPtr p( new boost::thread(&Active<M>::run, this ) );
            threads_[ p->get_id() ] = p;
        }
    }
    catch(...)
    {
        throw;
    }
}

//-----------------------------------------------------------------------------

template < typename M >
Active<M>::~Active()
{
    mq_.drain_and_close();
    done_ = true;

    // wait for all threads still processing queue messages to exit normally
    for( ThreadPool::iterator i = threads_.begin(); i != threads_.end(); ++i )
        if( i->second->joinable() )
            i->second->join();
}

//-----------------------------------------------------------------------------

template < typename M >
bool Active<M>::send( M msg )
{
    return mq_.wait_and_push(msg);
}

//-----------------------------------------------------------------------------

template < typename M >
void Active<M>::run()
{

//    std::cout << "> starting run()\n" << std::flush;

    M m;
    while (!done_)
    {
        try
        {
            if( mq_.try_and_pop(m) )
                dispatch_(m);
            else
                boost::this_thread::yield();
        }
        catch ( boost::thread_interrupted& e )
        {

            break; //< finish this thread
        }
    }

//    std::cout << "> ending run()" << std::flush;
}

//-----------------------------------------------------------------------------

#endif

