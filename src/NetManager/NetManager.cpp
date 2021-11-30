#include "infra/Logguer.hpp"
#include "NetManager.hpp"
#include "NetManager/Client.hpp"

#include <cassert>
#include <string>

static inline int uvcheck( const char *title, int retval ) {
    if( retval < 0 ) {
        LOG_ERROR( ) << uv_err_name( retval ) << ", " << uv_strerror( retval );
        throw std::runtime_error( title );
    }

    return retval;
}

static void globalAfterCloseCB( uv_handle_t *handle ) {
    auto baseClient = static_cast< infra::BaseClient * >( handle->data );
    auto client = static_cast< nm::Client * >( baseClient );

    LOG_INFO( ) << baseClient << " Cerrando socket";

    delete client;
}

NetManager::NetManager( ) :
    m_loop( uv_default_loop( ) )
{
    if( !m_loop ) {
        throw std::runtime_error( "uv_default_loop( )" );
    }
}

int NetManager::start( int port ) {
    LOG_INFO( ) << "Iniciando NetManager, puerto " << port;

    sockaddr_in addr;

    uvcheck( "Server uv_ip4_addr( )", uv_ip4_addr( "0.0.0.0", port, &addr ) );
    uvcheck( "Server uv_tcp_init( )", uv_tcp_init( m_loop, selfTcp( ) ) );
    selfHandle( )->data = this;
    uvcheck( "Server uv_tcp_bind( )", uv_tcp_bind( selfTcp( ), reinterpret_cast< sockaddr * >( &addr ), 0 ) );
    uvcheck( "Server uv_listen( )", uv_listen( selfStream( ), 15, globalOnNewConnection ) );

    LOG_INFO( ) << "Iniciando bucle de eventos";

    return uv_run( m_loop, UV_RUN_DEFAULT );
}

// SLOT
void NetManager::startUpload( BaseClient *client ) noexcept {
    auto realClient = static_cast< Client * >( client );
    LOG_DEBUG( ) << client << " Autorizado UPLOAD. Iniciando rececpión";
    realClient->startUpload( );
}

// SLOT
void NetManager::sendResult( BaseClient *client, std::string result ) noexcept {
    auto realClient = static_cast< Client * >( client );
    LOG_DEBUG( ) << client << " Enviando resultado";
    realClient->sendResult( std::move( result ) );
}

// SLOT
void NetManager::cancelClient( BaseClient *client ) noexcept {
    // Esto NO EMITE 'cancelSignal'.
    auto realClient = static_cast< Client * >( client );

    assert( realClient->canary( ) );
    uv_close( realClient->selfHandle( ), globalAfterCloseCB );
}

void NetManager::onClientError( Client *client ) noexcept {
    LOG_ERROR( ) << "Cliente [ " << client->remoteIP( ) << ':' << client->remotePort( ) << " ] " << client->error( ).toString( );
    cancelSignal( client );

    assert( client->canary( ) );
    uv_close( client->selfHandle( ), globalAfterCloseCB );
}

void NetManager::onClientDisconnected( Client *client ) noexcept {
    LOG_INFO( ) << "Cliente " << static_cast< BaseClient * >( client ) << " ha cerrado la conexión";
    cancelSignal( client );

    assert( client->canary( ) );
    uv_close( client->selfHandle( ), globalAfterCloseCB );
}

void NetManager::onReadyToUpload( Client *client ) noexcept {
    uploadReceivedSignal( static_cast< BaseClient * >( client ) );
}

void NetManager::onUploadFinished( Client *client, std::filesystem::path originalPath, infra::Buffer buffer ) noexcept {
    uploadFinishSignal( static_cast< BaseClient * >( client ), std::move( originalPath ), std::move( buffer ) );
}

void NetManager::onClientBadAlloc( Client *client ) noexcept {
    // De momento, solo cancelamos el cliente.
    cancelSignal( static_cast< BaseClient * >( client ) );

    assert( client->canary( ) );
    uv_close( client->selfHandle( ), globalAfterCloseCB );
}

void NetManager::onNewConnection( int status ) noexcept {
    if( status != 0 ) {
        LOG_ERROR( ) << uv_err_name( status ) << ", " << uv_strerror( status );
        return;
    }

    auto newClient = nm::Client::factory( m_loop, selfStream( ) );

    if( newClient ) {
        newClient->errorSignal.attach( this, &NetManager::onClientError );
        newClient->disconnectedSignal.attach( this, &NetManager::onClientDisconnected );
        newClient->readyToUploadSignal.attach( this, &NetManager::onReadyToUpload );
        newClient->uploadFinishedSignal.attach( this, &NetManager::onUploadFinished );
        newClient->badAllocSignal.attach( this, &NetManager::onClientBadAlloc );

        if( !newClient->start( ) ) {
            uv_close( newClient->selfHandle( ), globalAfterCloseCB );
        }
    }
}

void NetManager::globalOnNewConnection( uv_stream_t *server, int status ) noexcept {
    auto self = static_cast< NetManager * >( server->data );
    self->onNewConnection( status );
}
