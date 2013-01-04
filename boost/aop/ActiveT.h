#ifndef Active_h
#define Active_h

#define BOOST_THREAD_VERSION 3

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/future.hpp>

#include "message_queue.h"

//-----------------------------------------------------------------------------

template < typename M, typename R >
class Active : private boost::noncopyable {

public: // types

    class Exception {};

    typedef M message_type;
    typedef R return_type;

    typedef typename boost::promise<R>                   promise_type;
    typedef typename boost::shared_ptr< promise_type >   promise_ptr;
    typedef typename boost::future<R>                    future_type;

    typedef boost::function< return_type ( message_type ) > execution_type;

    typedef boost::function< void ( message_type, execution_type, promise_ptr ) > dispatcher_type;

private: // types

    typedef boost::function< void () > work_type;

    typedef boost::shared_ptr< boost::thread > ThreadPtr;
    typedef std::map< boost::thread::id, ThreadPtr > ThreadPool;

private: // data

    mutable boost::mutex      m_;             ///< mutex for locking changes in the Active object itself

    bool                      done_;          ///< flag for finishing

    execution_type            exec_;          ///< function to handle each message
    dispatcher_type           dispatcher_;    ///< dispacthes the execution and takes care of promises

    ThreadPool                threads_;       ///< multiple threads object

    message_queue<work_type>  mq_;            ///< message queue

public: // methods

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active( execution_type x, size_t nb_threads = 1, size_t qsize = 0 ) :
        done_(false),
        exec_(x),
        dispatcher_( boost::bind( &Active<M,R>::default_dispatch, this, _1, _2, _3 ) ),
        mq_(qsize)
    {
        try
        {
            for( size_t i = 0; i < nb_threads; ++i )
            {
                ThreadPtr p( new boost::thread(&Active<M,R>::run, this ) );
                threads_[ p->get_id() ] = p;
            }
        }
        catch(...)
        {
            throw;
        }
    }


    /// Destructor
    /// Enqueue done message and wait for queue to drain
    virtual ~Active();

    /// Enqueue a message
    promise_ptr send( M msg )
    {
        boost::lock_guard<boost::mutex> lock(m_);

        promise_ptr p ( new promise_type() );

        work_type w = boost::bind( dispatcher_, msg, exec_, p );

        if( ! mq_.wait_and_push( w ) )
            throw Active<M,R>::Exception();

        return p;
    }

    /// @returns the nb worker threads
    size_t tsize();

    /// @returns the current queue size
    size_t qsize() { return mq_.size(); }

    /// sets the maximum queue size
    void qsize( const size_t& s ) { mq_.max_size(s); }

    void dispatcher( dispatcher_type d )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        dispatcher_ = d;
    }

private: // methods

    void default_dispatch( message_type m, execution_type exec, promise_ptr p )
    {
        p->set_value( exec( m ) );
    }

    void run()
    {
        // std::cout << "> starting run()\n" << std::flush;
        work_type w;
        while (!done_)
        {
            try
            {
                if( mq_.try_and_pop(w) )
                    w();
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

};

//-----------------------------------------------------------------------------

template < typename M, typename R >
Active<M,R>::~Active()
{
    mq_.drain_and_close();
    done_ = true;

    // wait for all threads still processing queue messages to exit normally
    for( ThreadPool::iterator i = threads_.begin(); i != threads_.end(); ++i )
        if( i->second->joinable() )
            i->second->join();
}

//-----------------------------------------------------------------------------

#endif

