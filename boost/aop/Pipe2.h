#ifndef Pipe2_h
#define Pipe2_h

#include <boost/noncopyable.hpp>

#include "ActiveT.h"

//-----------------------------------------------------------------------------

template < typename AO >
class Pipe2Dispatcher {

public: // methods

    typedef void result_type; ///< for boost::function behavior

    typedef typename AO::message_type  pipe_input_t;
    typedef typename AO::result_type   pipe_output_t;

    typedef boost::shared_ptr< boost::promise<pipe_output_t> > pipe_promise_ptr;

    Pipe2Dispatcher( AO& o ) : sink_(o) {}

    template< typename M >
    void operator() ( boost::function< pipe_input_t ( M ) > exec, M m, pipe_promise_ptr p )
    {
        ///< M is a promise, exec waits for completion, unpacks & returns the value
        sink_.send( exec( m ), p);
    }

private: // members

    AO& sink_;

};

//-----------------------------------------------------------------------------

template< typename AO >
Pipe2Dispatcher<AO> dispatch_to( AO& o ) { return Pipe2Dispatcher<AO>(o); }

//-----------------------------------------------------------------------------

/// Pipe execution function
/// @param M is a promise for result from the source object
/// @returns the
template< typename M, typename R >
R wait_and_unpack_promise( boost::shared_ptr<boost::promise< M > > p )
{
    boost::future<M> f = p->get_future();
    f.wait();
    return f.get();
}

//-----------------------------------------------------------------------------

/// Pipe2 solution between Active objects
///
/// Dispatcher in each AOP remains the same
/// Pipes are typed source_type to sink_type
/// Pipes are ActiveObjects themselves
/// Threads will wait for the result from source and dispatch to sink

template < typename SOURCE, typename SINK >
class Pipe2 :
    private Active< typename SOURCE::promise_ptr, typename SINK::result_type > {

public: // types

    typedef SOURCE   source_type;
    typedef SINK     sink_type;

    typedef typename SOURCE::message_type message_type;
    typedef typename SINK::result_type    result_type;

    typedef typename boost::promise<result_type>         promise_type;
    typedef typename boost::shared_ptr< promise_type >   promise_ptr;
    typedef typename boost::future<result_type>          future_type;

protected: // types

    typedef typename SOURCE::promise_ptr  pipe_input_type;
    typedef typename SINK::result_type    pipe_output_type;

    typedef boost::function< typename SINK::message_type ( typename SOURCE::promise_type ) > execution_type;

    typedef Active< pipe_input_type, pipe_output_type > implementation_type;

    typedef typename implementation_type::work_type  work_type;

public: // methods

    Pipe2( source_type& source, sink_type& sink ) :
        implementation_type(
            &(wait_and_unpack_promise<typename SOURCE::result_type, typename SINK::message_type>), source.tsize(), 0, dispatch_to(sink) ),
        source_(source),
        sink_(sink)
    {
    }

    promise_ptr send( message_type msg )
    {
        /// enqueue immedietly the work in source_

        typename source_type::promise_ptr pin ( source_.send(msg) );

        /// create a promise for the result of this pipe

        promise_ptr pout( new promise_type() );

        boost::unique_lock<boost::mutex> lock( this->m_);

        work_type w = boost::bind( this->dispatch_, this->exec_, pin, pout );

        lock.unlock(); // unlock here, not to block in case we wait before pushing

        if( ! this->mq_.wait_and_push( w ) )
            throw typename implementation_type::Exception();

        return pout;
    }

private: // members

    source_type& source_;
    sink_type&   sink_;

};

template < typename T1, typename T2 >
Pipe2<T1,T2> make_pipe2( T1& source, T2& sink )
{
    return Pipe2<T1,T2>(source,sink);
}

//-----------------------------------------------------------------------------

#endif

