cmake_minimum_required( VERSION 2.8.3 FATAL_ERROR )

project( sandbox C CXX )

# include local cmake modules
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

find_package(Perl)

add_subdirectory( cpp )

if(PERL_FOUND)
  add_subdirectory( perl )
endif()
