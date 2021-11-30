#include "Crupier/Work.hpp"
#include "infra/BaseClient.hpp"

#include <chrono>
#include <thread>

namespace crup {

Work::Work( Crupier *c, BaseClient *bc ) :
    m_crupier( c ),
    m_baseClient( bc )
{
    m_baseClient->setUserData( this );
    m_cancelled.test_and_set( ); // El trabajo NO está cancelado.
}

void Work::start( ) {
    auto time = m_buffer.size( ) / 1000;
    if( time < 250 ) { time = 250; }

    std::this_thread::sleep_for( std::chrono::milliseconds( time ) );
}

} // namespace crup.
