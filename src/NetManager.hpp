#ifndef NETMANAGER_HPP
#define NETMANAGER_HPP

#include "infra/Buffer.hpp"
#include "infra/Signal.hpp"
#include "infra/Types.hpp"
#include "NetManager/UvTcp.hpp"

#include <filesystem>

class NetManager : public nm::UvTcp {
public:
    infra::Signal< BaseClient * > uploadReceivedSignal;
    infra::Signal< BaseClient *, std::filesystem::path, infra::Buffer > uploadFinishSignal;
    infra::Signal< BaseClient * > cancelSignal;

    infra::Signal< BaseClient * > badAllocSignal;

    NetManager( );

    int start( int port );

    // SLOTS.
    void startUpload( BaseClient *client ) noexcept;
    void sendResult( BaseClient *client, std::string result ) noexcept;
    void cancelClient( BaseClient *client ) noexcept;

private:
    void onClientError( Client * ) noexcept;
    void onClientDisconnected( Client * ) noexcept;
    void onReadyToUpload( Client * ) noexcept;
    void onUploadFinished( Client *, std::filesystem::path originalPath, infra::Buffer buffer ) noexcept;
    void onClientBadAlloc( Client * ) noexcept;

    void onNewConnection( int status ) noexcept;

    static void globalOnNewConnection( uv_stream_t *server, int status ) noexcept;

    uv_loop_t *m_loop;
};

#endif
