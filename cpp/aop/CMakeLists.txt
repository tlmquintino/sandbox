# TODO:
#      * add kill message to remove one thread
#      * add spawn message to spawn more threads
#      * return futures
#      * what to do to messages queued after the 'done' message?
#          - add a closed method to the queue class?
#          - how to return errors?

### active object with single worker thread
### suitable for wrapping around resources that need synchronized access

add_executable( cpp11_aop cpp11_aop.cc message_queue.h )

### active object with multiple worker threads
### usefull when messages work on data that isn't shared and needs no synchronized access

add_executable( cpp11_aop_mt cpp11_aop_mt.cc message_queue.h )
