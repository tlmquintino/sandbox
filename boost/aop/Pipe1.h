#ifndef Pipe1_h
#define Pipe1_h

#include <boost/noncopyable.hpp>

#include "ActiveT.h"

//-----------------------------------------------------------------------------

template < typename AO >
class Pipe1Dispatcher {

public: // methods

    typedef typename AO::result_type   pipe_output_t;
    typedef typename AO::message_type  pipe_input_t;

    typedef void result_type;
    typedef boost::shared_ptr< boost::promise<pipe_input_t> > promise_t;

    Pipe1Dispatcher( AO& o ) : sink_(o) {}

    template< typename M >
    void operator() ( boost::function< pipe_input_t ( M ) > exec, M m, promise_t p )
    {
        p->set_value( exec( m ) );
        sink_.send( p->get_future().get() );
    }

private: // members

    AO& sink_;

};

//-----------------------------------------------------------------------------

template< typename AO >
Pipe1Dispatcher<AO> make_pipe_to( AO& o ) { return Pipe1Dispatcher<AO>(o); }

//-----------------------------------------------------------------------------

/// Pipe1 solution between Active objects
///
/// Involves changing the dispatcher in each upstream AOP
/// Pipes are typed source_type to sink_type
/// Pipes don't return their result
/// Pipes are not ActiveObjects themselves

template < typename MS, typename PP, typename RS >
class Pipe1 : private boost::noncopyable {

public: // types

    typedef MS   message_type;
    typedef RS   result_type;
    typedef PP   transport_type;

    typedef Active<MS,PP> source_type;
    typedef Active<PP,RS> sink_type;

public: // methods

    Pipe1( source_type& source, sink_type& sink) :
        source_(source),
        sink_(sink)
    {
        source_.dispatcher( make_pipe_to( sink_ ) );
    }

    void send( message_type msg )
    {
        source_.send(msg);
    }

private: // members

    source_type&  source_;
    sink_type&    sink_;

};

//-----------------------------------------------------------------------------

template < typename T1, typename T2 >
Pipe1<
    typename T1::message_type,
    typename T1::result_type,
    typename T2::result_type >
    make_pipe1( T1& source, T2& sink )
{
    return Pipe1<
            typename T1::message_type,
            typename T1::result_type,
            typename T2::result_type > (source,sink);
}

//-----------------------------------------------------------------------------

#endif

