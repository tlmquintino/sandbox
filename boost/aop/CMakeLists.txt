# TODO:
#
#      * return futures ( boost::future & boost::promise )
#
#      * make a pipe chainning a group of active objs
#
#          - Active::run should now take f() and use it as a value for the action
#          - Active::run is then polymorphic ?
#          - example:
#              obj 1 - produces a series of integers [ 1 2 3 4 5 6 7 8 9 ]
#              obj 2 - multiplies them by 10
#              obj 3 - tee's them two ways to obj 4a & 4b
#              obj 4a - sums them in groups of 3
#              obj 4b - multiplies them by 2
#              obj 5  - prints them as they arrive
#
#                 /--(4a)--\
#     (1)--(2)--(3)        (5)
#                 \--(4b)--/
#
#              ---
#     (c)--(s)-|-- MPI
#              ---
#

### active object with single worker thread
### suitable for wrapping around resources that need synchronized access

add_executable( boost_aop boost_aop.cc shared_queue.h ActiveST.h )

target_link_libraries( boost_aop ${Boost_LIBRARIES} )

### active object with multiple worker threads
### usefull when messages work on data that isn't shared and needs no synchronized access

add_executable( boost_aop_mt boost_aop_mt.cc shared_queue.h  ActiveMT.h )

target_link_libraries( boost_aop_mt ${Boost_LIBRARIES} )

### active object with futures
###

add_executable( boost_aop_future boost_aop_future.cc shared_queue.h  ActiveFuture.h )

target_link_libraries( boost_aop_future ${Boost_LIBRARIES} )
