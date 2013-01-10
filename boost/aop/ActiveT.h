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

template < typename R >
struct Dispatcher
{
    typedef void result_type;
    typedef boost::shared_ptr< boost::promise<R> > promise_t;
    template< typename M >
    void operator() ( boost::function< R ( M ) > exec, M m, promise_t r )
    {
        r->set_value( exec( m ) );
    }
};

template <>
struct Dispatcher<void>
{
    typedef void result_type;
    typedef boost::shared_ptr< boost::promise<void> > promise_t;
    template< typename M >
    void operator() ( boost::function< void ( M ) > exec, M m, promise_t r )
    {
        exec( m );
    }
};

template < typename AO >
class PipeDispatcher {

public: // methods

    typedef typename AO::result_type   pipe_output_t;
    typedef typename AO::message_type  pipe_input_t;

    typedef void result_type;
    typedef boost::shared_ptr< boost::promise<pipe_input_t> > promise_t;

    PipeDispatcher( AO& o ) : sink_(o) {}

    template< typename M >
    void operator() ( boost::function< pipe_input_t ( M ) > exec, M m, promise_t p )
    {
        p->set_value( exec( m ) );
        sink_.send( p->get_future().get() );
    }

private: // members

    AO& sink_;

};

template< typename AO >
PipeDispatcher<AO> make_pipe_to( AO& o ) { return PipeDispatcher<AO>(o); }

//-----------------------------------------------------------------------------

template < typename M, typename R >
class Active : private boost::noncopyable {

public: // types

    class Exception {};

    typedef M message_type;
    typedef R result_type;

    typedef typename boost::promise<result_type>         promise_type;
    typedef typename boost::shared_ptr< promise_type >   promise_ptr;
    typedef typename boost::future<result_type>          future_type;

    typedef boost::function< result_type ( message_type ) > execution_type;

    typedef boost::function< void ( execution_type, message_type, promise_ptr ) > dispatcher_type;

private: // types

    typedef boost::function< void () > work_type;

    typedef boost::shared_ptr< boost::thread > ThreadPtr;

    typedef std::map< boost::thread::id, ThreadPtr > ThreadPool;

private: // data

    mutable boost::mutex      m_;             ///< mutex for locking changes in the Active object itself

    bool                      done_;          ///< flag for finishing

    execution_type            exec_;          ///< function to handle each message

    ThreadPool                threads_;       ///< multiple threads object

    message_queue<work_type>  mq_;            ///< message queue

    dispatcher_type           dispatch_;      ///< dispatcher of tasks

public: // methods

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active( execution_type x,
            size_t nb_threads = 1,
            size_t qsize = 0,
            dispatcher_type d = Dispatcher<result_type>() ) :
        done_(false),
        exec_(x),
        mq_(qsize),
        dispatch_( d )
    {
        spawn_threads(nb_threads);
    }

    /// Destructor
    /// Enqueue done message and wait for queue to drain
    virtual ~Active()
    {
        mq_.drain_and_close();
        done_ = true;

        // wait for all threads still processing queue messages to exit normally
        for( ThreadPool::iterator i = threads_.begin(); i != threads_.end(); ++i )
            if( i->second->joinable() )
                i->second->join();
    }

    /// Enqueue a message
    promise_ptr send( message_type msg )
    {
        promise_ptr p ( new promise_type() );

        boost::unique_lock<boost::mutex> lock(m_);

        work_type w = boost::bind( dispatch_, exec_, msg, p );

        lock.unlock(); // unlock here, not to block in case we wait before pushing

        if( ! mq_.wait_and_push( w ) )
            throw Active<M,R>::Exception();

        return p;
    }

    /// Sets the dispatcher
    void dispatcher( dispatcher_type d )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        dispatch_ = d;
    }

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

private: // methods

    void spawn_threads( size_t nb_threads )
    {
        boost::lock_guard<boost::mutex> lock(m_);
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

#endif

