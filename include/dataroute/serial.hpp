#pragma once

#include <string>
#include <vector>

namespace dataroute {

class SerialPort {
public:
  SerialPort();
  ~SerialPort();

  SerialPort(const SerialPort& other) = delete;
  SerialPort(SerialPort&& other);
  SerialPort& operator=(const SerialPort& other) = delete;
  SerialPort& operator=(SerialPort&& other);

  void connect(const std::string& port, int baudrate, int timeout_deciseconds = 1);
  bool connected() const;

  const std::vector<std::uint8_t>& read(std::size_t num_bytes);
  void write(const std::vector<std::uint8_t>& bytes);

private:
  int fd_;
  std::vector<std::uint8_t> buffer_;
};

} // namespace dataroute
