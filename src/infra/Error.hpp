#ifndef INFRA_ERROR_HPP
#define INFRA_ERROR_HPP

#include <string>

namespace infra {

class Error {
public:
    enum Value {
        NO_ERROR,
        INTERNAL,
        BAD_ALLOC,
        BAD_HEADER,
        NET_ERROR,
        BREAK_WAIT,
        BAD_UPLOAD,
        BUSY,
        WRITE,
        LAST_ERROR = BUSY
    };

    Value value;

    Error( Value val = NO_ERROR ) : value( val ) { }
    Error &operator=( const Error & ) noexcept = default;
    Error &operator=( Value val ) noexcept {
        value = val;
        return *this;
    }

    bool operator==( Value val ) const noexcept { return value == val; }
    bool operator!=( Value val ) const noexcept { return value != val; }
    bool operator==( const Error &other ) const noexcept { return value == other.value; }
    bool operator!=( const Error &other ) const noexcept { return value != other.value; }

    const char *toString( ) const noexcept { return Error::toString( value ); }

    static inline const char *toString( Value val ) noexcept {
        int intValue = static_cast< int >( val );

        if( ( val < NO_ERROR ) || ( val > LAST_ERROR ) ) { intValue = INTERNAL; }

        return m_errorMsgs[intValue];
    }

private:
    static inline const char *m_errorMsgs[] = {
        "Ok, no error",
        "Error interno",
        "Memoria insuficiente",
        "Error al interpretar HELLO",
        "Error de red",
        "Cliente envío durante la espera",
        "Cliente envío demasiados datos durante UPLOAD"
        "Error al escribir. Socket ocupado",
        "Error al escribir en socket"
    };
};

} // namespace infra.

#endif
