#pragma once

#include <span>
#include <cstdint>
#include <optional>
#include <vector>


namespace dataroute {

static constexpr std::array<std::uint8_t, 2> sync_bytes = { 0xA2, 0xB4 }; // Arbitrary

struct Packet {
    std::uint16_t id;
    std::span<const std::uint8_t> payload;
};

bool decode_packet(const std::span<const std::uint8_t>& buffer, Packet& packet);

void encode_packet(const Packet& packet, std::vector<std::uint8_t>& buffer);
bool encode_packet(const Packet& packet, std::uint8_t* buffer, std::size_t& size, std::size_t capacity);

} // namespace dataroute
