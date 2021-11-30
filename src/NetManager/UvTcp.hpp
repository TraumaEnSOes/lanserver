#ifndef NETMANAGER_UVTCP_HPP
#define NETMANAGER_UVTCP_HPP

#include <uv.h>

namespace nm {

class UvTcp {
public:
    uv_handle_t *selfHandle( ) noexcept { return reinterpret_cast< uv_handle_t * >( &m_uv ); }

protected:
    const uv_handle_t *selfHandle( ) const noexcept { return reinterpret_cast< const uv_handle_t * >( &m_uv ); }
    const uv_handle_t *cselfHandle( ) const noexcept { return reinterpret_cast< const uv_handle_t * >( &m_uv ); }
    uv_stream_t *selfStream( ) noexcept { return reinterpret_cast< uv_stream_t * >( &m_uv ); }
    const uv_stream_t *selfStream( ) const noexcept { return reinterpret_cast< const uv_stream_t * >( &m_uv ); }
    const uv_stream_t *cselfStream( ) const noexcept { return reinterpret_cast< const uv_stream_t * >( &m_uv ); }
    uv_tcp_t *selfTcp( ) noexcept { return &m_uv; }
    const uv_tcp_t *selfTcp( ) const noexcept { return &m_uv; }
    const uv_tcp_t *cselfTcp( ) const noexcept { return &m_uv; }

private:
    uv_tcp_t m_uv;
};

} // namespace nm.

#endif
