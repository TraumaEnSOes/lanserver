#ifndef CRUPIER_WORK_HPP
#define CRUPIER_WORK_HPP

// #include "Crupier/Interactors.hpp"
#include "infra/Buffer.hpp"
#include "infra/Types.hpp"

#include <atomic>
#include <filesystem>
#include <string>

#include <uv.h>

class Crupier;

namespace crup {

class Work {
public:
    Work( Crupier *, BaseClient *cli );

    Crupier *crupier( ) noexcept { return m_crupier; }
    BaseClient *baseClient( ) noexcept { return m_baseClient; }
    uv_work_t *uvwork( ) noexcept { return &m_work; }
    const uv_work_t *uvwork( ) const noexcept { return &m_work; }
    const uv_work_t *cuvwork( ) const noexcept { return &m_work; }

    void start( );
    void cancel( ) noexcept { m_cancelled.clear( ); }
    bool cancelled( ) noexcept { return !( m_cancelled.test_and_set( ) ); }

private:
    Crupier *m_crupier;
    BaseClient *m_baseClient;
    volatile std::atomic_flag m_cancelled;

    std::filesystem::path m_remotePath;
    infra::Buffer m_buffer;
    std::filesystem::path m_localPath;
    std::string m_result;
    uv_work_t m_work;

    infra::Error m_workError = infra::Error::NO_ERROR;
};

} // namespace crup.

#endif
