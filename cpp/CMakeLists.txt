option( ENABLE_CPP11 "build c++11 stuff" ON )

if( ENABLE_CPP11 )

if( CMAKE_CXX_COMPILER_ID MATCHES "GNU"   OR 
    CMAKE_CXX_COMPILER_ID MATCHES "Intel" OR 
    CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    list( APPEND CMAKE_CXX_FLAGS "-std=c++0x" )
#    list( APPEND CMAKE_CXX_FLAGS "-std=c++11" )
endif()

add_subdirectory( auto )
add_subdirectory( thread )

endif()
