cmake_minimum_required( VERSION 3.21.3 )
project( LanAccess CXX )

if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Debug )
endif( )

set( ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}" )

set( TMP_BUFFER_SIZE 16384U )
set( HEADER_BUFFER_SIZE 80192U )

add_compile_options( -Wall -Wextra -pedantic )

configure_file( "config.h.in" "config.h" )

add_subdirectory( src )
#add_subdirectory( tests )
