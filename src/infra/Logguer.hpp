#ifndef INFRA_LOGGUER_HPP
#define INFRA_LOGGUER_HPP

#include "infra/BaseClient.hpp"

#include <filesystem>
#include <iostream>
#include <map>

namespace logger {

class Tail {
public:
    ~Tail( ) {
        std::cout << std::endl;
    }

    template< typename T > Tail &operator<<( const T &data ) {
        std::cout << data;

        return *this;
    }

    Tail &operator<<( infra::BaseClient *client ) {
        std::cout << '[' << client->remoteIP( ) << ':' << client->remotePort( ) << ']';
        return *this;
    }
};

struct Head {
    Head( const char *file, const char *funct, int line, char type ) {
        auto fileName = std::filesystem::path( file ).filename( ).string( );

        if( type == 'T' ) {
            std::cout << fileName << '#' << funct << ':' << line << " TRACE" << std::endl;
        } else {
            std::cout << '[' << type << "] " << fileName << ':' << line << ' ';
        }
    }
    template< typename T > Tail operator<<( const T &data ) {
        std::cout << data;

        return Tail( );
    }
    Tail operator<<( infra::BaseClient *client ) {
        std::cout << '[' << client->remoteIP( ) << ':' << client->remotePort( ) << ']';
        return Tail( );
    }
};

struct Empty {
    Empty( ) { }
    template< typename T > Empty operator<<( const T & ) { return Empty( ); }
    Empty operator<<( infra::BaseClient * ) { return Empty( ); }
};

} // namespace logger.

#define LOG_ERROR( ) logger::Head( __FILE__, __func__, __LINE__, 'E' )
//#define LOG_ERROR( ) logger::Empty( )
#define LOG_WARN( ) logger::Head( __FILE__, __func__, __LINE__, 'W' )
//#define LOG_WARN( ) logger::Empty( )
#define LOG_INFO( ) logger::Head( __FILE__, __func__, __LINE__, 'I' )
//#define LOG_INFO( ) logger::Empty( )
// #define LOG_DEBUG( ) logger::Head( __FILE__, __func__, __LINE__, 'G' )
#define LOG_DEBUG( ) logger::Empty( )

#endif
