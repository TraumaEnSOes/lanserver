#include "config.h"

#include "infra/Logguer.hpp"
#include "NetManager/Client.hpp"

#include <cassert>

static uv_buf_t AllocGlobalMemoryBuffer( );

static uv_buf_t GlobalTmpBuffer = AllocGlobalMemoryBuffer( );

static std::string OK( "OK" );

static uv_buf_t AllocGlobalMemoryBuffer( ) {
    uv_buf_t result;

    result.base = new char[TMP_BUFFER_SIZE];
    result.len = TMP_BUFFER_SIZE;

    return result;
}

namespace nm {

bool Client::start( ) noexcept {
    if( auto retval = uv_read_start( selfStream( ), globalAllocCB, globalOnReadCB ); retval < 0 ) {
        LOG_ERROR( ) << "uv_read_start( )" << uv_err_name( retval ) << ", " << uv_strerror( retval );

        return false;
    }

    sockaddr peerName{ };
    int nameLen = sizeof( peerName );
    if( auto retval = uv_tcp_getpeername( selfTcp( ), &peerName, &nameLen ); retval != 0 ) {
        LOG_WARN( ) << "uv_tcp_getpeername( )" << uv_err_name( retval ) << ", " << uv_strerror( retval );
        LOG_INFO( ) << "Nueva conexión establecida";
    } else {
        auto in = reinterpret_cast< sockaddr_in * >( &peerName );
        m_remoteIP = inet_ntoa( in->sin_addr );
        m_remotePort = ntohs( in->sin_port );

        LOG_INFO( ) << static_cast< BaseClient * >( this ) << " Establecida conexión";
    }

    receiveHeader( );

    return m_error == Error::NO_ERROR ? true : false;
}

void Client::onUploadInteractor( infra::Buffer data ) noexcept {
    auto baseClient = static_cast< BaseClient * >( this );
    LOG_DEBUG( ) << baseClient << " Finalizado UPLOAD de " << data.size( ) << " byte";
    LOG_DEBUG( ) << baseClient << " Notificando ruta remota " << m_originalPath << " y " << data.size( ) << " bytes";
    uploadFinishedSignal( this, std::move( m_originalPath ), std::move( m_buffer ) );
}

void Client::onHeaderInteractor( UploadSize size, std::filesystem::path path ) noexcept {
    m_uploadSize = size;
    m_originalPath = std::move( path );

    readyToUploadSignal( this );
}

void Client::receiveHeader( ) noexcept {
    LOG_DEBUG( ) << static_cast< BaseClient * >( this ) << " Estableciendo HeaderInteractor";

    m_interactorStore = HeaderInteractor( );
    m_interactor = &std::get< HeaderInteractor >( m_interactorStore );
}

void Client::startUpload( ) noexcept {
    m_sendBuffer.clear( );

    sendBuffer( Client::globalStartUploadCB );
}

void Client::sendResult( std::string msg ) noexcept {
    m_sendBuffer = std::move( msg );
    sendBuffer( Client::globalStartHeaderCB );
}

Client *Client::factory( uv_loop_t *loop, uv_stream_t *server ) noexcept {
    std::unique_ptr< Client > tmp( new (std::nothrow) Client( ) );

    if( !tmp ) {
        LOG_ERROR( ) << "new( )";
        return nullptr;
    }

    if( auto retval = uv_tcp_init( loop, tmp->selfTcp( ) ); retval != 0 ) {
        LOG_ERROR( ) << "uv_tcp_init( )" << uv_err_name( retval ) << ", " << uv_strerror( retval );
        return nullptr;
    }

    if( auto retval = uv_accept( server, tmp->selfStream( ) ); retval != 0 ) {
        LOG_ERROR( ) << "uv_accept( ): " << uv_err_name( retval ) << ", " << uv_strerror( retval );
        uv_close( tmp->selfHandle( ), nullptr );
        return nullptr;
    }

    // A partir de aquí, el socket está conectado.
    // El destructor debe cerrar la conexión.
    tmp->m_connectionTime = uv_now( loop );
    tmp->m_remotePort = 0;
    tmp->selfHandle( )->data = tmp.get( );

    return tmp.release( );
}

void Client::onRead( ssize_t nread, const uv_buf_t *buf ) noexcept {
   if( nread == 0 ) {
        // No hay datos para leer.
        return;
    }
    if( nread == UV_EOF ) {
        // Cierre del socket.
        disconnectedSignal( this );
        return;
    }
    if( nread < 0 ) {
        // Error de red.
        m_error = Error::NET_ERROR;
        errorSignal( this );
        return;
    }

    std::string_view data( buf->base, nread );
    bool continuable = m_interactor->execute( data );

    if( !continuable ) {
        m_error = m_interactor->error( );

        if( m_error == Error::NO_ERROR ) {
            m_interactor->notify( this, &Client::onHeaderInteractor, &Client::onUploadInteractor );
        } else {
            errorSignal( this );
        }

        LOG_DEBUG( ) << static_cast< BaseClient * >( this ) << " Estableciendo WaitInteractor";
        m_interactorStore = WaitInteractor( );
        m_interactor = &std::get< WaitInteractor >( m_interactorStore );
    }
}

void Client::sendBuffer( void (*cb)( uv_write_t *req, int status ) ) noexcept {
    assert( cb );

    uv_buf_t buffer;

    if( m_sendBuffer.empty( ) ) {
        // Es para recibir el primer HEADER.
        LOG_DEBUG( ) << "Enviando Ok\\00";

        buffer.base = OK.data( );
        buffer.len = OK.size( ) + 1;
    } else {
        // Es una Respuesta.
        LOG_DEBUG( ) << "Enviando " << m_sendBuffer << "\\00";

        buffer.base = m_sendBuffer.data( );
        buffer.len = m_sendBuffer.size( ) + 1U;
    }

    if( auto retval = uv_write( &m_writeRequest, selfStream( ), &buffer, 1U, cb ); retval != 0 ) {
        m_error = Error::WRITE;
        errorSignal( this );
    }

    m_writeRequest.data = this;
}

void Client::globalOnReadCB( uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf ) noexcept {
    auto self = static_cast< Client * >( stream->data );
    self->onRead( nread, buf );
}

void Client::globalAllocCB( uv_handle_t *, size_t, uv_buf_t *buf ) noexcept {
    *buf = GlobalTmpBuffer;
}

void Client::globalStartHeaderCB( uv_write_t *req, int status ) noexcept {
    auto self = reinterpret_cast< Client * >( req->data );

    if( status ) {
        self->m_error = Error::WRITE;
        self->errorSignal( self );
    } else {
        LOG_DEBUG( ) << self << " Estableciendo HeaderInteractor";

        self->m_sendBuffer.clear( );
        self->m_interactorStore = HeaderInteractor( );
        self->m_interactor = &std::get< HeaderInteractor >( self->m_interactorStore );
    }
}

void Client::globalStartUploadCB( uv_write_t *req, int status ) noexcept {
    assert( req->data );
    auto self = reinterpret_cast< Client * >( req->data );
    assert( self->m_uploadSize );

    if( status ) {
        self->m_error = Error::WRITE;
        self->errorSignal( self );
        return;
    } else {
        LOG_DEBUG( ) << static_cast< BaseClient * >( self ) << " Estableciendo UploadInteractor";

        self->m_sendBuffer.clear( );
        self->m_interactorStore = UploadInteractor( self->m_uploadSize );
        self->m_interactor = &std::get< UploadInteractor >( self->m_interactorStore );
    }
}

} // namespace nm.
