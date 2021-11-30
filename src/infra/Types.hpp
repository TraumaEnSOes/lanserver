#ifndef INFRA_TYPES_HPP
#define INFRA_TYPES_HPP

#include "infra/Error.hpp"
#include <cstddef>

namespace nm {

class Client;

} // namespace nm.

namespace infra {

class BaseClient;
class BaseWork;

} // namespace infra.

namespace crup {

class Work;

} // namespace crup.

using Time = uint64_t;
using Unsigned = size_t;
using UploadSize = size_t;
using UploadTotalSize = unsigned long long;
using Error = infra::Error;
using BaseClient = infra::BaseClient;
using Client = nm::Client;
using BaseWork = infra::BaseWork;
using Work = crup::Work;

#endif
