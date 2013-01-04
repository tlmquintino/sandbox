#ifndef shared_queue_h
#define shared_queue_h

#include <queue>
#include <exception>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>
#include <boost/noncopyable.hpp>

template<typename T>
class shared_queue : private boost::noncopyable {

private: // data

    bool   open_;     ///< if queue is accepting more entries
    size_t max_;      ///< maximum size of queue

    std::queue<T> queue_;
    mutable boost::mutex m_;
    boost::condition_variable data_cond_;

public: // methods

  shared_queue( size_t max = 0 ) :
      open_(true),
      max_(max)
  {
  }

  ~shared_queue()
  {
//      std::cout << "!! queue size [" << size() << "]" << std::endl;
  }

  bool wait_and_push( T item )
  {
      boost::unique_lock<boost::mutex> lock(m_);

      if( open_ )
      {
          if( max_ )
          {
            while( queue_.size() >= max_ )
            {
              data_cond_.wait(lock);
            }
          }

          queue_.push(item);
          data_cond_.notify_one();

      }
      return open_;
  }

  /// Forces an entry into the queue irrespective of size or open status
  void push( T item )
  {
    boost::lock_guard<boost::mutex> lock(m_);
    queue_.push(item);
    data_cond_.notify_one();
  }

  bool try_and_pop(T& item)
  {
    boost::lock_guard<boost::mutex> lock(m_);
    if(queue_.empty()){
      return false;
    }
    item = queue_.front();
    queue_.pop();
    data_cond_.notify_one();
    return true;
  }

  void wait_and_pop(T& item)
  {
    boost::unique_lock<boost::mutex> lock(m_);
    while(queue_.empty())
    {
      data_cond_.wait(lock);
    }
    item = queue_.front();
    queue_.pop();
    data_cond_.notify_one();
  }

  void open()
  {
      boost::unique_lock<boost::mutex> lock(m_);
      if(!open_)
          open_ = true;
  }

  void drain_and_close()
  {
      boost::unique_lock<boost::mutex> lock(m_);
      open_ = false; // don't accept more entries
      while( !queue_.empty() )
      {
        data_cond_.wait(lock);
      }
      data_cond_.notify_one();
  }

  bool is_open()
  {
    boost::lock_guard<boost::mutex> lock(m_);
    return open_;
  }

  bool empty() const
  {
    boost::lock_guard<boost::mutex> lock(m_);
    return queue_.empty();
  }

  size_t size() const
  {
    boost::lock_guard<boost::mutex> lock(m_);
    return queue_.size();
  }

  void max_size( const size_t& s )
  {
    boost::lock_guard<boost::mutex> lock(m_);
    max_ = s;
  }

};

#endif
