# TODO:
#      * add kill message to remove one thread
#      * add spawn message to spawn more threads
#      * return futures
#      * what to do to messages queued after the 'done' message?
#          - add a closed method to the queue class?
#          - how to return errors?

### active object with single worker thread
### suitable for wrapping around resources that need synchronized access

add_executable( boost_aop boost_aop.cc shared_queue.h )

target_link_libraries( boost_aop ${Boost_LIBRARIES} )

### active object with multiple worker threads
### usefull when messages work on data that isn't shared and needs no synchronized access

add_executable( boost_aop_mt boost_aop_mt.cc shared_queue.h )

target_link_libraries( boost_aop_mt ${Boost_LIBRARIES} )
