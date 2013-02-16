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

template < typename M >
class IActive : private boost::noncopyable {

public: // types

    typedef boost::shared_ptr< IActive<M> > Ptr;

    typedef M message_type;

public: // interface

    virtual void send( M msg ) = 0;

};

template <>
class IActive<void> : private boost::noncopyable {

public: // types

    typedef boost::shared_ptr< IActive<void> > Ptr;

public: // interface

};

//-----------------------------------------------------------------------------

template < typename M >
class Pipe : public IActive<M> {

public: // types

    typedef boost::shared_ptr< Pipe<M> > Ptr;

    typedef M message_type;

public: // interface

    virtual void send( M msg )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        for( size_t i = 0; i < sinks_.size(); ++i )
            sinks_[i]->send( msg );
    }

    void add_sink( typename IActive<M>::Ptr sink )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        sinks_.push_back(sink);
    }

protected: // data

    mutable boost::mutex      m_;           ///< mutex for locking changes

    std::vector< typename IActive<M>::Ptr > sinks_;  ///< pipe destinations
};

//-----------------------------------------------------------------------------

namespace detail {

template < typename R >
struct Dispatcher
{
    typedef void result_type;
    typedef boost::shared_ptr< boost::promise<R> > promise_t;

    template< typename M >
    void operator() ( typename IActive<R>::Ptr p, boost::function< R ( M ) > exec, M m, promise_t pr )
    {
        R r = exec( m );
        pr->set_value( r );
        p->send( r );
    }

    template< typename M >
    void operator() ( typename IActive<R>::Ptr p, boost::function< R ( M ) > exec, M m )
    {
        p->send( exec( m ) );
    }
};

template <>
struct Dispatcher<void>
{
    typedef void result_type;
    typedef boost::shared_ptr< boost::promise<void> > promise_t;

    template< typename M >
    void operator() ( typename IActive<void>::Ptr p, boost::function< void ( M ) > exec, M m, promise_t pr )
    {
        exec( m );
    }

    template< typename M >
    void operator() ( typename IActive<void>::Ptr p, boost::function< void ( M ) > exec, M m )
    {
        exec( m );
    }
};

}

//-----------------------------------------------------------------------------

template < typename M, typename R >
class Active : public IActive<M> {

public: // types

    typedef boost::shared_ptr< Active<M,R> > Ptr;

    class Exception {};

    typedef M message_type;
    typedef R result_type;

    typedef typename boost::promise<result_type>         promise_type;
    typedef typename boost::shared_ptr< promise_type >   promise_ptr;
    typedef typename boost::future<result_type>          future_type;

    typedef boost::function< result_type ( message_type ) > execution_type;

    typedef typename IActive<result_type>::Ptr  pipe_type;

protected: // types

    typedef boost::function< void () > work_type;

    typedef boost::shared_ptr< boost::thread > thread_ptr;

    typedef std::map< boost::thread::id, thread_ptr > ThreadPool;

    typedef typename detail::Dispatcher<result_type> dispatcher_type;

protected: // data

    mutable boost::mutex      m_;             ///< mutex for locking changes in the Active object itself

    bool                      done_;          ///< flag for finishing

    execution_type            exec_;          ///< function to handle each message

    ThreadPool                threads_;       ///< multiple threads object

    message_queue<work_type>  mq_;            ///< message queue

    dispatcher_type           dispatch_;      ///< dispatcher of tasks

    pipe_type                 pipe_;          ///< possible holds a pipe

public: // methods

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active( execution_type x,
            size_t nb_threads = 1,
            size_t qsize = 0 ) :
        done_(false),
        exec_(x),
        mq_(qsize),
        dispatch_(),
        pipe_()
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
    virtual void send( message_type msg )
    {
        boost::unique_lock<boost::mutex> lock(m_);

        work_type w = boost::bind( dispatch_, pipe_, exec_, msg );

        lock.unlock();

        if( ! mq_.wait_and_push( w ) )
            throw Active<M,R>::Exception();
    }

    /// Enqueues a message
    /// @returns on promise passed from outsides
    virtual void send( message_type msg, promise_ptr p )
    {
        boost::unique_lock<boost::mutex> lock(m_);

        work_type w = boost::bind( dispatch_, pipe_, exec_, msg, p );

        lock.unlock();

        if( ! mq_.wait_and_push( w ) )
            throw Active<M,R>::Exception();
    }

    void pipe( const pipe_type& p )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        return pipe_.reset(p);
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

    /// Factory method
    static Active<M,R>::Ptr create( execution_type x, size_t nb_threads = 1, size_t qsize = 0 )
    {
        return Active<M,R>::Ptr( new Active<M,R>(x,nb_threads,qsize) );
    }

protected: // methods

    void spawn_threads( size_t nb_threads )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        try
        {
            for( size_t i = 0; i < nb_threads; ++i )
            {
                thread_ptr p( new boost::thread(&Active<M,R>::run, this ) );
                threads_[ p->get_id() ] = p;
            }
        }
        catch(...)
        {
            throw;
        }
    }

    virtual void run()
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

/// Operator to pipe Active objects to other Active objects
template< typename T1, typename T2 >
typename IActive<typename T2::message_type>::Ptr
operator| ( boost::shared_ptr<T1> source, boost::shared_ptr<T2> sink )
{
    typedef Pipe< typename T2::message_type > pipe_t;

    pipe_t* pp = new pipe_t();
    pp->add_sink(sink);

    typename T1::pipe_type p ( pp );
    source->pipe( p );

    return boost::dynamic_pointer_cast< typename IActive<typename T2::message_type>::Ptr >(sink);
}

//-----------------------------------------------------------------------------

#endif

