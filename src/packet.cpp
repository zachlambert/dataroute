#include "dataroute/packet.hpp"
#include <cstring>
#include <cstdio>

namespace dataroute {

std::uint8_t calculate_checksum(const Packet& packet) {
    std::uint8_t result = 0;
    for (auto byte: packet.payload) {
        result += byte;
    }
    result += (std::uint8_t)(packet.id >> 0);
    result += (std::uint8_t)(packet.id >> 8);
    return ~result;
}

static constexpr std::size_t packet_header_size =
    sync_bytes.size()
    + sizeof(std::uint16_t)
    + sizeof(std::uint64_t);

inline std::size_t packet_size(const Packet& packet) {
    return packet_header_size + packet.payload.size() + 1;
}

bool decode_packet(const std::span<const std::uint8_t>& buffer, Packet& packet) {
    if (buffer.size() < packet_header_size + 1) {
        return false;
    }

    std::size_t pos = 0;
    for (std::size_t i = 0; i < sync_bytes.size(); i++) {
        if (sync_bytes[i] != buffer[pos + i]) {
            return false;
        }
    }
    pos += sync_bytes.size();

    packet.id = *(const std::uint16_t*)&buffer[pos];
    pos += sizeof(std::uint16_t);

    std::uint64_t payload_size = *(const std::uint64_t*)&buffer[pos];
    pos += sizeof(std::uint64_t);

    packet.payload = std::span(&buffer[pos], payload_size);
    pos += payload_size;

    std::uint8_t checksum = buffer[pos];
    if (checksum != calculate_checksum(packet)) {
        return false;
    }

    return true;
}


void encode_to_buffer(const Packet& packet, std::uint8_t* buffer) {
    std::size_t pos = 0;
    std::memcpy((void*)&buffer[pos], &sync_bytes[0], sync_bytes.size());
    pos += sync_bytes.size();

    *(std::uint16_t*)(&buffer[pos]) = packet.id;
    pos += sizeof(std::uint16_t);

    *(std::uint64_t*)(&buffer[pos]) = packet.payload.size();
    pos += sizeof(std::uint64_t);

    std::memcpy((void*)&buffer[pos], packet.payload.data(), packet.payload.size());
    pos += packet.payload.size();

    buffer[pos] = calculate_checksum(packet);
}

void encode_packet(const Packet& packet, std::vector<std::uint8_t>& buffer) {
    std::size_t required_size = packet_size(packet);
    buffer.resize(required_size);
    encode_to_buffer(packet, &buffer[0]);
}

bool encode_packet(
        const Packet& packet,
        std::uint8_t* buffer,
        std::size_t& size,
        std::size_t capacity)
{
    std::size_t required_size = packet_size(packet);
    if (required_size > capacity) {
        return false;
    }

    size = required_size;
    encode_to_buffer(packet, buffer);
    return true;
}

} // namespace datapack
