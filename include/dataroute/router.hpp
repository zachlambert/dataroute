#pragma once

#include <functional>
#include <memory>
#include <type_traits>

#include <datapack/datapack.hpp>
#include <datapack/format/binary_reader.hpp>
#include <datapack/format/binary_writer.hpp>

#include "dataroute/packet.hpp"
#include "dataroute/interface.hpp"


namespace dataroute {

class Router {
    struct CallbackBase {
        virtual void call(const std::span<const std::uint8_t>& payload) const = 0;
    };

    template <datapack::readable T>
    struct Callback: public CallbackBase {
        std::function<void(const T&)> callback;

        Callback(const std::function<void(const T&)>& callback):
            callback(callback)
        {}
        void call(const std::span<const std::uint8_t>& payload) const override {
            callback(datapack::read_binary<T>(payload));
        }
    };

public:
    template <typename T>
    requires std::is_base_of_v<Interface, T>
    Router(T&& interface):
        interface(std::make_unique<T>(interface))
    {}

    template <datapack::readable T>
    void add_callback(std::uint16_t id, const std::function<void(const T&)>& callback) {
        callbacks.emplace(id, std::make_unique<Callback<T>>(callback));
    }

    void poll() {
        while (true) {
            std::span<const std::uint8_t> bytes = interface->read();
            if (bytes.empty()) return;

            Packet packet;
            if (!decode_packet(bytes, packet)) continue;
            auto iter = callbacks.find(packet.id);
            if (iter == callbacks.end()) continue;
            iter->second->call(packet.payload);
        }
    }

    template <datapack::writeable T>
    void write(std::uint16_t id, const T& value) {
        std::vector<std::uint8_t> payload = datapack::write_binary(value);
        Packet packet;
        packet.id = id;
        packet.payload = payload;
        std::vector<std::uint8_t> buffer;
        encode_packet(packet, buffer);
        interface->write(buffer);
    }

private:
    std::unordered_map<std::uint16_t, std::unique_ptr<CallbackBase>> callbacks;
    std::unique_ptr<Interface> interface;
};

} // namespace dataroute
