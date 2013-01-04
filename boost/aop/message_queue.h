#ifndef message_queue_h
#define message_queue_h

#include <queue>
#include <exception>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>
#include <boost/noncopyable.hpp>

template<typename T>
class message_queue : private boost::noncopyable {

private: // data

    bool   open_;           ///< if queue is accepting more entries
    size_t max_;            ///< maximum size of queue

    std::queue<T> queue_;   ///< actual queue storage

    mutable boost::mutex m_;                ///< mutex for the queue's resource
    boost::condition_variable data_cond_;   ///< condition variable for the queue resource

public: // methods

    /// Constructor
    /// @param max size of the queue, 0 is unlimited
    message_queue( size_t max = 0 ) :
        open_(true),
        max_(max)
    {
    }

    /// Destructor
    ~message_queue()
    {
    }

    /// Wait (blocking) until there is free space in the queue and the push an item
    /// @returns true if item was pushed, false if queue was closed and item not pushed
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
    /// This means that queue may sometimes have more items than its max value
    void push( T item )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        queue_.push(item);
        data_cond_.notify_one();
    }

    /// Try to pop an item (blocking)
    /// @returns true if item was poped, false if queue was empty and no item was poped
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

    /// Wait (block) until there is an item to pop
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

    /// Open's the queue if it wasn't open
    void open()
    {
        boost::unique_lock<boost::mutex> lock(m_);
        if(!open_)
            open_ = true;
    }

    /// Waits for queue to be emptied, then closes it
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

    /// @returns if queue is open
    bool is_open()
    {
        boost::lock_guard<boost::mutex> lock(m_);
        return open_;
    }

    /// @returns if queue is empty
    bool empty() const
    {
        boost::lock_guard<boost::mutex> lock(m_);
        return queue_.empty();
    }

    /// @returns current size of the queue
    size_t size() const
    {
        boost::lock_guard<boost::mutex> lock(m_);
        return queue_.size();
    }

    /// @param maximum size of queue to set
    void max_size( const size_t& s )
    {
        boost::lock_guard<boost::mutex> lock(m_);
        max_ = s;
    }

};

#endif
