option( ENABLE_BOOST "build boost stuff" ON )

if( ENABLE_BOOST )

set( Boost_USE_MULTITHREAD ON )
set( Boost_NO_SYSTEM_PATHS ON )

find_package( Boost 1.50 COMPONENTS date_time filesystem system thread )

if( CMAKE_CXX_COMPILER_ID MATCHES "GNU"   OR
    CMAKE_CXX_COMPILER_ID MATCHES "Intel" OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    list( APPEND CMAKE_CXX_FLAGS "-std=c++11" )
endif()

if( Boost_FOUND )
	
    message( STATUS "Boost_LIBRARIES   [${Boost_LIBRARIES}]" )
    message( STATUS "Boost_INCLUDE_DIR [${Boost_INCLUDE_DIR}]" )

	include_directories( ${Boost_INCLUDE_DIR} )
	
	add_subdirectory( asio     )
	add_subdirectory( optional )
	add_subdirectory( thread   )
	add_subdirectory( activeobjs )

endif()

endif()
