#include "Crupier.hpp"
#include "NetManager.hpp"

#include <iostream>
#include <string>

static inline int parsePort( const char *arg ) {
    std::string puerto( arg );

    return std::stoi( puerto );
}

int main( int argc, char **argv ) {
    std::ios::sync_with_stdio( false );

    if( argc != 2 ) {
        std::cout << "Uso: server PUERTO\n";
        return 1;
    }

    NetManager netManager;
    Crupier crupier;

    netManager.uploadReceivedSignal.attach( &crupier, &Crupier::onEnqueueUpload );
    netManager.uploadFinishSignal.attach( &crupier, &Crupier::onUploadFinish );
    netManager.cancelSignal.attach( &crupier, &Crupier::onCancel );

    crupier.startUploadSignal.attach( &netManager, &NetManager::startUpload );
    crupier.sendResultSignal.attach( &netManager, &NetManager::sendResult );
    // crupier.onUploadFinish.attach( &netManager, &NetManager::)
    // crupier.sendResultSignal.attach( &netManager, &NetManager::sendResult );
    // crupier.syncStartSignal.attach( &syncEmitter, &AuthSyncEmitter::launchSync );

    return netManager.start( parsePort( argv[1] ) );
}
