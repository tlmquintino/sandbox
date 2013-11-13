option( ENABLE_BOOST "build boost stuff" ON )

if( ENABLE_BOOST )

  set( Boost_USE_MULTITHREAD ON )
  set( Boost_NO_SYSTEM_PATHS ON )

  find_package( Boost 1.50 COMPONENTS date_time chrono filesystem system thread )

  if( Boost_FOUND )

    message( STATUS "Boost_LIBRARIES   [${Boost_LIBRARIES}]" )
    message( STATUS "Boost_INCLUDE_DIR [${Boost_INCLUDE_DIR}]" )

	include_directories( ${Boost_INCLUDE_DIR} )

	add_subdirectory( asio     )
	add_subdirectory( optional )
	add_subdirectory( thread   )
	add_subdirectory( any )
	# add_subdirectory( aop )
	#	add_subdirectory( monads )
	add_subdirectory( expressions )

  endif()

endif()
