#ifndef CRUPIER_HPP
#define CRUPIER_HPP

#include "infra/Buffer.hpp"
#include "infra/Logguer.hpp"
#include "infra/Signal.hpp"
#include "infra/Types.hpp"
#include "Crupier/SyncEmitter.hpp"

#include <queue>
#include <string>

typedef struct uv_async_s uv_async_t;

class Crupier {
public:
    using Queue = std::queue< Work * >;

    infra::Signal< BaseClient * > startUploadSignal;
    infra::Signal< BaseClient *, std::string > sendResultSignal;

    Crupier( Unsigned uploadFactor = 2U );

    // PUBLIC SLOTS
    void onEnqueueUpload( BaseClient *client ) noexcept;
    void onUploadFinish( BaseClient *client, std::filesystem::path originalPath, infra::Buffer buffer ) noexcept;
    void onCancel( BaseClient *client ) noexcept;

private:
    // PRIVATE SLOTS
    void onSync( ) noexcept;
    void onWorkFinish( Work *work ) noexcept;

    static void globalExecuteWorkCB( uv_work_t *req ) noexcept;
    static void globalAfterWorkCB( uv_work_t *req, int status ) noexcept;

    uv_loop_t *m_loop;
    crup::SyncEmitter< > m_syncEmitter;

    Unsigned m_totalWorkers;
    Unsigned m_maxReadyToProcess;
    Unsigned m_activeWorks = 0;
    Unsigned m_activeUploads = 0;

    Queue m_readyToUploadQueue;
    Queue m_readyToProcessQueue;
    Queue m_readytoSendResultQueue;
};

#endif
