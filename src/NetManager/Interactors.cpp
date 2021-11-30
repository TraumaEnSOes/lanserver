#include "config.h"

#include "infra/Logguer.hpp"
#include "NetManager/Client.hpp"
#include "NetManager/Interactors.hpp"

#include <algorithm>

static constexpr char ZERO = '\0';

namespace nm {

Interactor::~Interactor( ) {
}

bool WaitInteractor::execute( std::string_view ) noexcept {
    m_error = Error::BREAK_WAIT;
    return false;
}

void WaitInteractor::notify( Client *, void (Client::*)( UploadSize, std::filesystem::path ), void (Client::*)( infra::Buffer ) ) noexcept {
    // Esto NO PUEDE PASAR.
    return;
}

HeaderInteractor::HeaderInteractor( ) :
    Interactor( "Header" )
{
    try {
        m_buffer.reserve( HEADER_BUFFER_SIZE );
    } catch( const std::bad_alloc & ) {
        m_error = Error::BAD_ALLOC;
    }
}

bool HeaderInteractor::execute( std::string_view data ) noexcept {
    if( ( m_buffer.size( ) + data.size( ) ) > HEADER_BUFFER_SIZE ) {
        LOG_ERROR( ) << "Datos de HEADER demasiado grandes";

        m_error = Error::BAD_HEADER;
        return false;
    }

    m_buffer.insert( m_buffer.end( ), data.cbegin( ), data.cend( ) );

    if( !m_first.has_value( ) ) {
        auto iter = std::find( m_buffer.cbegin( ), m_buffer.cend( ), ZERO );

        if( iter == m_buffer.cend( ) ) {
            return true;
        }

        m_first = iter + 1;
    }

    auto iter = std::find( m_first.value( ), m_buffer.cend( ), ZERO );

    if( iter == m_buffer.cend( ) ) {
        // Solo 1 ZERO. Datos incompletos.
        return true;
    } else if( ( iter + 1 ) != m_buffer.cend( ) ) {
        m_error = Error::BAD_HEADER;
        return false;
    }

    m_size = strtoll( &( *m_buffer.cbegin( ) ), nullptr, 10 );

    if( m_size < 1 ) {
        LOG_ERROR( ) << "Tamaño de archivo inválido";
        m_error = Error::BAD_HEADER;
        return false;
    }

    try {
        m_path = std::string_view( &( **m_first ) );
    } catch( const std::bad_alloc & ) {
        m_error = Error::BAD_ALLOC;
        return false;
    }

    if( m_path.empty( ) ) {
        LOG_ERROR( ) << "Ruta de archivo vacía";
        m_error = Error::BAD_HEADER;
        return false;
    }

    if( m_path.is_relative( ) ) {
        LOG_ERROR( ) << "Ruta de archivo relativa";
        m_error = Error::BAD_HEADER;
        return false;
    }

    LOG_DEBUG( ) << "Hello correcto, " << m_size << " bytes, " << m_path.string( );
    return false;
}

void HeaderInteractor::notify( Client *client, void (Client::*slot)( UploadSize, std::filesystem::path ), void (Client::*)( infra::Buffer ) ) noexcept {
    (client->*slot)( m_size, std::move( m_path ) );
}

UploadInteractor::UploadInteractor( UploadSize size ) :
    Interactor( "Upload" ),
    m_size( size )
{
}

bool UploadInteractor::execute( std::string_view data ) noexcept {
    if( data.empty( ) ) {
        return true;
    }

    if( ( m_size - m_buffer.size( ) ) < data.size( ) ) {
        m_error = Error::BAD_UPLOAD;
        return false;
    }

    m_buffer.insert( m_buffer.begin( ), data.cbegin( ), data.cend( ) );

    return m_buffer.size( ) == m_size ? false : true;
}

void UploadInteractor::notify( Client *client, void (Client::*)( UploadSize, std::filesystem::path ), void (Client::*slot)( infra::Buffer ) ) noexcept {
    (client->*slot)( std::move( m_buffer ) );
}

} // namespace.
