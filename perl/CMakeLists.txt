option( ENABLE_PERL "build boost stuff" ON )

if( ENABLE_PERL )

find_package(Perl)

if(PERL_FOUND)
    add_subdirectory( oo.v1 )
    add_subdirectory( oo.v2 )
endif()

endif()
