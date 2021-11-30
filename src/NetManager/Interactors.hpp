#ifndef NETMANAGER_INTERACTORS_HPP
#define NETMANAGER_INTERACTORS_HPP

#include "infra/Buffer.hpp"
#include "infra/Signal.hpp"
#include "infra/Types.hpp"

#include <filesystem>
#include <optional>
#include <string_view>

namespace nm {

class Client;

class Interactor {
public:
    virtual ~Interactor( );
    Interactor( const char *type ) : m_type( type ) { }

    virtual bool execute( std::string_view data ) noexcept = 0;
    virtual void notify( Client *client,
                         void (Client::*)( UploadSize, std::filesystem::path ),
                         void (Client::*)( infra::Buffer )
                        ) noexcept = 0;

    Error error( ) const noexcept { return m_error; }

protected:
    Error m_error = Error::NO_ERROR;

private:
    const char *m_type;
};

class WaitInteractor : public Interactor {
public:
    WaitInteractor( ) : Interactor( "Wait" ) { }

    bool execute( std::string_view data ) noexcept override;
    void notify( Client *client,
                 void (Client::*)( UploadSize, std::filesystem::path ),
                 void (Client::*)( infra::Buffer )
                ) noexcept override;
};

class HeaderInteractor : public Interactor {
public:
    HeaderInteractor( );

    bool execute( std::string_view data ) noexcept override;
    void notify( Client *client,
                 void (Client::*)( UploadSize, std::filesystem::path ),
                 void (Client::*)( infra::Buffer )
                ) noexcept override;

private:
    UploadSize m_size = 0;
    std::optional< std::vector< char >::const_iterator > m_first;
    infra::Buffer m_buffer;
    std::filesystem::path m_path;
};

class UploadInteractor : public Interactor {
public:
    UploadInteractor( UploadSize size );

    bool execute( std::string_view data ) noexcept override;
    void notify( Client *client,
                 void (Client::*)( UploadSize, std::filesystem::path ),
                 void (Client::*)( infra::Buffer )
                ) noexcept override;

private:
    UploadSize m_size;
    infra::Buffer m_buffer;
};

} // namespace nm

#endif
