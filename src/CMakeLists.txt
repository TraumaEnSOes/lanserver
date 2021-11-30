find_package( PkgConfig REQUIRED )
find_package( Threads REQUIRED )

if( LIBUV_PATH )
    pkg_check_modules( Uv REQUIRED IMPORTED_TARGET "${LIBUV_PATH}" )
else( )
    pkg_check_modules( Uv REQUIRED IMPORTED_TARGET libuv )
endif( )

include( cmakefiles.cmake )
add_executable( server ${SOURCES} )

set_target_properties( server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

set_target_properties( server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories( server
    PRIVATE "${ROOT_DIR}/src"
    PRIVATE PkgConfig::Uv
    PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries( server
    PRIVATE PkgConfig::Uv
)
