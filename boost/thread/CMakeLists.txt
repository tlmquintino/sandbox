add_executable( boost_thread boost_thread.cc )

target_link_libraries( boost_thread ${Boost_LIBRARIES} )

add_executable( boost_future boost_future.cc )

target_link_libraries( boost_future ${Boost_LIBRARIES} )
