#ifndef CRUPIER_SYNCEMITTER_HPP
#define CRUPIER_SYNCEMITTER_HPP

#include "infra/Logguer.hpp"

#include <uv.h>

#include <functional>

namespace crup {

namespace details {

class SyncEmitter {
protected:
    SyncEmitter( uv_loop_t *loop ) {
        if( auto retval = uv_async_init( loop, &m_async, &SyncEmitter::globalOnSyncCB ); retval != 0 ) {
            throw std::runtime_error( uv_strerror( retval ) );
        }
    }

protected:
    void launchSync( ) noexcept {
        m_calleed = true;
        if( auto retval = uv_async_send( &m_async ); retval != 0 ) {
            LOG_ERROR( ) << uv_err_name( retval ) << ": " << uv_strerror( retval );
            throw std::runtime_error( "uv_async_send( )" );
        }

        m_async.data = this;
    }

private:
    virtual void onSyncCB( ) noexcept = 0;

    static void globalOnSyncCB( uv_async_t *async ) noexcept {
        auto self = reinterpret_cast< SyncEmitter * >( async->data );

        self->m_calleed = false;
        self->onSyncCB( );
    }

    bool m_calleed = false;
    uv_async_t m_async;
};

} // namespace details.

template< typename T = void > class SyncEmitter : public details::SyncEmitter {
public:
    SyncEmitter( uv_loop_t *loop ) : details::SyncEmitter( loop ) { }

    template< typename RET > void attach( RET (*cb)( T ) ) noexcept {
        m_cb = cb;
    }

    template< typename RET, typename CLASS > void attach( CLASS *instance, RET (CLASS::*member)( T ) ) noexcept {
        auto lambda = [instance, member]( T arg ) -> void { (instance->*member)( std::move( arg ) ); };

        m_cb = lambda;
    }

    void dtach( ) noexcept {
        m_cb = std::function< void( T ) >( );
    }

    void launchSync( T value ) {
        m_value = std::move( value );

        details::SyncEmitter::launchSync( );
    }

private:
    void onSyncCB( ) noexcept override {
        if( m_cb ) {
            m_cb( std::move( m_value ) );
        }
    }

    std::function< void( T ) > m_cb;
    T m_value;
};

template< > class SyncEmitter< void > : public details::SyncEmitter {
public:
    SyncEmitter( uv_loop_t *loop ) : details::SyncEmitter( loop ) { }

    template< typename RET > void attach( RET (*cb)( ) ) noexcept {
        m_cb = cb;
    }

    template< typename RET, typename CLASS > void attach( CLASS *instance, RET (CLASS::*member)( ) ) noexcept {
        auto lambda = [instance, member]( ) -> void { (instance->*member)( ); };

        m_cb = lambda;
    }

    void dtach( ) noexcept {
        m_cb = std::function< void( ) >( );
    }

    void launchSync( ) {
        details::SyncEmitter::launchSync( );
    }

private:
    void onSyncCB( ) noexcept override {
        if( m_cb ) {
            m_cb( );
        }
    }

    std::function< void( ) > m_cb;
};

} // namespace crup.

#endif
