#ifndef NETMANAGER_CLIENT_HPP
#define NETMANAGER_CLIENT_HPP

#include "infra/BaseClient.hpp"
#include "infra/Signal.hpp"
#include "infra/Types.hpp"
#include "NetManager/Interactors.hpp"
#include "NetManager/UvTcp.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <variant>

namespace nm {

class Client : public infra::BaseClient, public UvTcp {
public:
    using Error = infra::Error;
    using BaseClient = infra::BaseClient;

    infra::Signal< Client * > errorSignal;
    infra::Signal< Client * > badAllocSignal;
    infra::Signal< Client * > disconnectedSignal;
    infra::Signal< Client * > readyToUploadSignal;
    infra::Signal< Client *, std::filesystem::path, infra::Buffer > uploadFinishedSignal;

    uint64_t connectionTime( ) const noexcept { return m_connectionTime; }
    Error error( ) const noexcept { return m_error; }

    bool start( ) noexcept;

    // Recibe la cabecera.
    void receiveHeader( ) noexcept;
    // Acepta datos.
    void startUpload( ) noexcept;
    // Envía el OK + resultado de último work (si lo hubiera), y recibe datos.
    void sendResult( std::string msg ) noexcept;

    [[nodiscard]] static Client *factory( uv_loop_t *loop, uv_stream_t *server ) noexcept;

    bool canary( ) const noexcept { return m_canary; }

private:
    // PRIVATE SLOTS.
    void onUploadInteractor( infra::Buffer data ) noexcept;
    void onHeaderInteractor( UploadSize size, std::filesystem::path path ) noexcept;

    void onRead( ssize_t nread, const uv_buf_t *buf ) noexcept;

    void sendBuffer( void (*cb)( uv_write_t *req, int status ) ) noexcept;

    static void globalOnReadCB( uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf ) noexcept;
    static void globalAllocCB( uv_handle_t *handle, size_t suggestedSize, uv_buf_t *buf ) noexcept;
    static void globalStartHeaderCB( uv_write_t *req, int status ) noexcept;
    static void globalStartUploadCB( uv_write_t *req, int status ) noexcept;

    std::variant< std::monostate, WaitInteractor, HeaderInteractor, UploadInteractor > m_interactorStore;

    Time m_connectionTime;
    Error m_error = Error::NO_ERROR;
    UploadSize m_uploadSize;
    std::filesystem::path m_originalPath;
    infra::Buffer m_buffer;
    Interactor *m_interactor;
    uv_write_t m_writeRequest;
    std::string m_sendBuffer;

    bool m_canary = true;
};

} // namespace nm.

#endif
