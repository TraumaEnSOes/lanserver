#ifndef INFRA_BUFFER_HPP
#define INFRA_BUFFER_HPP

#include <vector>

namespace infra {

class Buffer : public std::vector< char > {
public:
    Buffer( ) = default;
    Buffer( const Buffer & ) = delete;
    Buffer( Buffer && ) = default;

    Buffer &operator=( Buffer && ) = default;
};

} // namespace infra.

#endif
