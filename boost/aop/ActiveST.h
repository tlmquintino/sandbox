#ifndef Active_h
#define Active_h

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

//-----------------------------------------------------------------------------

typedef boost::function<void()> Message;

//-----------------------------------------------------------------------------

class Active : private boost::noncopyable {

private: // methods

    /// Constructor
    /// Starts up everything, using run as the thread mainline
    Active();

    /// Flags the
    void finish(){ done_ = true; }
    void run();

private: // data

    shared_queue<Message>      mq_;        ///< message queue
    bool                       done_;      ///< flag for finishing
    boost::thread              thd_;       ///< thread object

public: // methods

    /// Destructor
    /// Enqueue done message and wait for queue to drain
    virtual ~Active();

    /// Enqueue a message
    void send( Message msg );

    /// Factory -- construction & thread start
    static boost::shared_ptr<Active> create();

};

//-----------------------------------------------------------------------------

Active::Active(): done_(false)
{
}

Active::~Active()
{
  Message finish_msg = boost::bind( &Active::finish, this );
  // enqueue finish message
  send(finish_msg);
  // wait for all processing in queue
  thd_.join();
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
    mq_.wait_and_pop(f);
    f();
  }
  std::cout << "> ending run()" << std::endl;
}

//-----------------------------------------------------------------------------

boost::shared_ptr<Active> Active::create()
{
    boost::shared_ptr<Active> pao( new Active() );
    pao->thd_ = boost::thread(&Active::run, pao.get());
    return pao;
}

#endif
