if( CMAKE_COMPILER_IS_GNUCXX )
    list( APPEND CMAKE_CXX_FLAGS "-std=c++0x" )
endif()

add_executable( auto auto.cc )
