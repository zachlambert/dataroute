#pragma once

#include "dataroute/packet.hpp"

namespace dataroute {

class Interface {
public:
    virtual std::span<const std::uint8_t> read() = 0;
    virtual void write(const std::span<const std::uint8_t>& bytes) = 0;
};

} // namespace dataroute
