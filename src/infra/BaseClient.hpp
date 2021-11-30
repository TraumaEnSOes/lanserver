#ifndef INFRA_BASECLIENT_HPP
#define INFRA_BASECLIENT_HPP

#include "infra/Signal.hpp"

#include <string>

namespace infra {

struct BaseClient {
public:
    Signal< BaseClient * > beforeDestroySignal;

    const std::string remoteIP( ) const noexcept { return m_remoteIP; }
    int remotePort( ) const noexcept { return m_remotePort; }

    void setUserData( void *ud ) noexcept { m_userData = ud; }
    void *userData( ) noexcept { return m_userData; }
    const void *userData( ) const noexcept { return m_userData; }
    const void *cuserData( ) const noexcept { return m_userData; }

protected:
    std::string m_remoteIP;
    int m_remotePort = -1;
    void *m_userData = nullptr;
};

} // namespace infra.

#endif
