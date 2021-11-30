find_package( PkgConfig REQUIRED )
find_package( Threads REQUIRED )

if( LIBUV_PATH )
    pkg_check_modules( Uv REQUIRED IMPORTED_TARGET "${LIBUV_PATH}" )
else( )
    pkg_check_modules( Uv REQUIRED IMPORTED_TARGET libuv )
endif( )

include( cmakefiles.cmake )

add_executable( tests ${TESTS_SOURCES} )

set_target_properties( tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories( tests
    PRIVATE "${ROOT_DIR}/src"
    PRIVATE "${ROOT_DIR}/tests"
    PRIVATE PkgConfig::Uv
)

target_link_libraries( tests
    PRIVATE PkgConfig::Uv
)
