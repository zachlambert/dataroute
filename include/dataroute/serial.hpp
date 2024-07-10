#pragma once

#include <string>
#include <vector>
#include "dataroute/interface.hpp"


namespace dataroute {

class SerialPort: Interface {
public:
    SerialPort();
    ~SerialPort();

    SerialPort(const SerialPort& other) = delete;
    SerialPort(SerialPort&& other);
    SerialPort& operator=(const SerialPort& other) = delete;
    SerialPort& operator=(SerialPort&& other);

    void connect(const std::string& port, int baudrate, int timeout_deciseconds = 1);
    bool connected() const;

    std::span<const std::uint8_t> read() override;
    void write(const std::span<const std::uint8_t>& bytes) override;

private:
    int fd_;
    std::vector<std::uint8_t> buffer_;
    std::size_t read_size;
};

} // namespace dataroute
