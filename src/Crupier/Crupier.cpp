#include "Crupier.hpp"
#include "Crupier/Work.hpp"
#include "infra/Logguer.hpp"

#include <uv.h>

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <thread>

Crupier::Crupier( Unsigned uploadFactor ) :
    m_loop( uv_default_loop( ) ),
    m_syncEmitter( m_loop )
{
    assert( uploadFactor );

    m_totalWorkers = std::thread::hardware_concurrency( );
    m_maxReadyToProcess = m_totalWorkers * uploadFactor;

    if( setenv( "UV_THREADPOOL_SIZE", std::to_string( m_totalWorkers ).data( ), 1 ) == -1 ) {
        throw std::runtime_error( "setenv( UV_THREADPOOL_SIZE )" );
    }

    m_syncEmitter.attach( this, &Crupier::onSync );

    LOG_INFO( ) << "Iniciando Crupier, " << m_totalWorkers << " workers, cola de uploads listos: " << m_maxReadyToProcess;
}

// SLOT
void Crupier::onEnqueueUpload( BaseClient *client ) noexcept {
    assert( client );

    LOG_DEBUG( ) << client << " Encolando UPLOAD";

    auto work = static_cast< Work * >( client->userData( ) );
    if( !client->userData( ) ) {
        work = new Work( this, client );
        client->setUserData( work );
    }

    m_readyToUploadQueue.push( work );

    m_syncEmitter.launchSync( );
}

// SLOT
void Crupier:: onUploadFinish( BaseClient *client, std::filesystem::path, infra::Buffer ) noexcept {
    assert( client );
    auto work = static_cast< Work * >( client->userData( ) );
    assert( work );

    LOG_DEBUG( ) << client << " Finalizado UPLOAD, encolando";

    m_readyToProcessQueue.push( work );
    m_syncEmitter.launchSync( );
    --m_activeUploads;
}

void Crupier::onCancel( BaseClient *client ) noexcept {
    auto work = static_cast< Work * >( client->userData( ) );
    assert( work );

    LOG_INFO( ) << client << " Cancelando trabajo";

    if( work ) {
        work->cancel( );
    }
}

// SLOT
void Crupier::onSync( ) noexcept {
    LOG_DEBUG( ) << "Procesando tareas pendientes";

    // Enviamos todas las respuestas generadas.
    auto limit = m_readytoSendResultQueue.size( );
    for( Unsigned idx = 0; idx < limit; ++idx ) {
        auto work = m_readytoSendResultQueue.front( );
        m_readytoSendResultQueue.pop( );

        assert( work );

        if( work->cancelled( ) ) {
            delete work;
            continue;
        }

        auto baseClient = work->baseClient( );
        LOG_DEBUG( ) << baseClient << " Iniciado envio de respuesta";
        LOG_INFO( ) << baseClient << " Enviando respuesta OK 1000";
        sendResultSignal( baseClient, "OK 1000" );
    }

    // Arrancamos nuevas tareas.
    limit = std::min( ( m_totalWorkers * 2U ) - m_activeWorks, m_readyToProcessQueue.size( ) );
    assert( limit <= ( m_totalWorkers * 2U ) );
    for( Unsigned idx = 0; idx < limit; ++idx ) {
        auto work = m_readyToProcessQueue.front( );
        m_readyToProcessQueue.pop( );

        assert( work );
        assert( work->crupier( ) );

        if( work->cancelled( ) ) {
            // El cliente fue cancelado.
            delete work;
            continue;
        }

        // Iniciamos trabajos.
        auto baseClient = work->baseClient( );
        LOG_INFO( ) << baseClient << " Iniciando trabajo";
        work->uvwork( )->data = work;
        if( auto retval = uv_queue_work( m_loop, work->uvwork( ), &Crupier::globalExecuteWorkCB, &Crupier::globalAfterWorkCB ); retval != 0 ) {
            LOG_ERROR( ) << "Error al ejecutar trabajo";
        }
    }


    // Autorizamos nuevos UPLOADs.
    limit = std::min( m_maxReadyToProcess - m_activeUploads, m_readyToUploadQueue.size( ) );
    assert( limit < m_maxReadyToProcess );
    for( Unsigned idx = 0; idx < limit; ++idx ) {
        auto work = m_readyToUploadQueue.front( );
        m_readyToUploadQueue.pop( );

        assert( work );
        if( work->cancelled( ) ) {
            // El cliente fue cancelado.
            delete work;
            continue;
        }

        ++m_activeUploads;
        auto baseClient = work->baseClient( );
        LOG_INFO( ) << baseClient << " Autorizado UPLOAD";
        startUploadSignal( baseClient );
    }

    LOG_DEBUG( ) << "Fin del procesando de tareas pendientes";
}

void Crupier::globalExecuteWorkCB( uv_work_t *req ) noexcept {
    auto work = static_cast< Work * >( req->data );
    assert( work );

    work->start( );
}

void Crupier::globalAfterWorkCB( uv_work_t *req, int /*status*/ ) noexcept {
    auto work = static_cast< Work * >( req->data );
    assert( work );
    auto crupier = work->crupier( );
    assert( crupier );
    auto baseClient = work->baseClient( );
    assert( baseClient );

    LOG_DEBUG( ) << baseClient << " Finalizado trabajo";
    crupier->m_readytoSendResultQueue.push( work );

    crupier->m_syncEmitter.launchSync( );
}
