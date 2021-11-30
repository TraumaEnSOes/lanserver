#ifndef INFRA_SIGNAL_HPP
#define INFRA_SIGNAL_HPP

#include <functional>
#include <utility>

namespace infra {

template< typename... ARGS > class Signal {
public:
    Signal( ) = default;
    Signal( const Signal< ARGS... > & ) = delete;
    Signal &operator=( const Signal< ARGS... > & ) = delete;

    void attach( std::function< void( ARGS... ) > cb ) {
        m_cb = std::move( cb );
    }

    template< typename CLASS, typename RET > void attach( CLASS *instance, RET (CLASS::*member)( ARGS... ) ) {
        auto lambda = [instance, member]( ARGS... args ) -> void {
            (instance->*member)( std::forward< ARGS >( args )... );
        };

        m_cb = lambda;
    }

    void attach( Signal< ARGS... > &other ) {
        auto lambda = [&other]( ARGS... args ) -> void {
            other( std::forward< ARGS >( args )... );
        };

        m_cb = lambda;
    }

    void dtach( ) {
        m_cb = std::function< void( ARGS... ) >( );
    }

    void operator()( ARGS... args ) const {
        if( m_cb ) {
            m_cb( std::forward< ARGS >( args )... );
        }
    }

private:
    std::function< void( ARGS... ) > m_cb;
};

template< > class Signal< void > {
public:
    Signal( ) = default;
    Signal( const Signal< void > & ) = delete;
    Signal &operator=( const Signal< void > & ) = delete;

    void attach( std::function< void( ) > cb ) {
        m_cb = std::move( cb );
    }

    template< typename CLASS, typename RET > void attach( CLASS *instance, RET (CLASS::*member)( ) ) {
        auto lambda = [instance, member]( ) -> void {
            (instance->*member)( );
        };

        m_cb = lambda;
    }

    void attach( Signal< void > &other ) {
        auto lambda = [&other]( ) -> void {
            other( );
        };

        m_cb = lambda;
    }

    void dtach( ) {
        m_cb = std::function< void( ) >( );
    }

    void operator()( ) const {
        if( m_cb ) {
            m_cb( );
        }
    }

private:
    std::function< void( ) > m_cb;
};

} // namespace infra.

#endif
