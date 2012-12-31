option( ENABLE_UNIX "build unix stuff" ON )

if( ENABLE_UNIX )

add_subdirectory( file_locking )
add_subdirectory( file_size )
add_subdirectory( memory_exhaust )
add_subdirectory( mmap )
add_subdirectory( signal_handling )
add_subdirectory( signals_ipc )

endif()