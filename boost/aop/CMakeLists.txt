# TODO:
#      * return futures

### active object with single worker thread
### suitable for wrapping around resources that need synchronized access

add_executable( boost_aop boost_aop.cc shared_queue.h ActiveST.h )

target_link_libraries( boost_aop ${Boost_LIBRARIES} )

### active object with multiple worker threads
### usefull when messages work on data that isn't shared and needs no synchronized access

add_executable( boost_aop_mt boost_aop_mt.cc shared_queue.h  ActiveMT.h )

target_link_libraries( boost_aop_mt ${Boost_LIBRARIES} )
