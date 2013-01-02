#ifndef shared_queue_h
#define shared_queue_h

#include <queue>
#include <exception>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>
#include <boost/noncopyable.hpp>

template<typename T>
class shared_queue : private boost::noncopyable
{
    size_t max_;

    std::queue<T> queue_;
    mutable boost::mutex m_;
    boost::condition_variable data_cond_;

public:

  shared_queue( size_t max = 0 ) : max_(max){}

  void wait_and_push( T item )
  {
      if( max_ )
      {
        boost::unique_lock<boost::mutex> lock(m_);
        while( queue_.size() >= max_ )
        {
          data_cond_.wait(lock);
        }
        queue_.push(item);
        data_cond_.notify_one();
      }
      else
          push(item);
  }

  void push(T item)
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

  bool empty() const
  {
    boost::lock_guard<boost::mutex> lock(m_);
    return queue_.empty();
  }

  unsigned size() const
  {
    boost::lock_guard<boost::mutex> lock(m_);
    return queue_.size();
  }
};

#endif
