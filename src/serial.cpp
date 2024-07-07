#include "dataroute/serial.hpp"

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdexcept>


namespace dataroute {

static constexpr int CLOSED_FD = -1;

SerialPort::SerialPort():
  fd_(CLOSED_FD)
{}

SerialPort::~SerialPort()
{
  if (fd_ != CLOSED_FD) {
    close(fd_);
    fd_ = CLOSED_FD;
  }
}

SerialPort::SerialPort(SerialPort&& other):
  fd_(other.fd_),
  buffer_(std::move(other.buffer_))
{
  other.fd_ = CLOSED_FD;
}

SerialPort& SerialPort::operator=(SerialPort&& other)
{
  fd_ = other.fd_;
  other.fd_ = CLOSED_FD;
  buffer_ = std::move(other.buffer_);
  return *this;
}

void SerialPort::connect(const std::string& port, int baudrate, int timeout_deciseconds)
{
  fd_ = open(port.c_str(), O_RDWR);
  if (fd_ == CLOSED_FD) {
    throw std::runtime_error("Failed to open serial port");
  }

  // See for details:
  // https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

  struct termios2 tty;

  // Memset to zero, to set all flags to zero by default
  memset(&tty, 0, sizeof(tty));

  tty.c_cflag |= CS8;
  tty.c_cflag |= CREAD | CLOCAL;

  tty.c_cflag |= CBAUDEX;
  tty.c_ispeed = baudrate;
  tty.c_ospeed = baudrate;

  // For read(), VMIN = bytes to wait for, VTIME = deciseconds to wait for
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = timeout_deciseconds;

  if (ioctl(fd_, TCSETS2, &tty) != 0) {
    close(fd_);
    fd_ = CLOSED_FD;
    throw std::runtime_error("Failed to configure serial port");
  }
}

bool SerialPort::connected() const
{
  return fd_ != CLOSED_FD;
}

const std::vector<std::uint8_t>& SerialPort::read(std::size_t num_bytes)
{
  if (!connected()) {
    throw std::runtime_error("Tried to read from closed port");
  }

  buffer_.resize(num_bytes);
  int rx_bytes = ::read(fd_, buffer_.data(), buffer_.size());
  if (rx_bytes < 0) {
    throw std::runtime_error("Error occurred when reading from serial port");
  }
  buffer_.resize(rx_bytes);
  return buffer_;
}

void SerialPort::write(const std::vector<std::uint8_t>& bytes)
{
  if (!connected()) {
    throw std::runtime_error("Tried to write to closed port");
  }

  int tx_bytes = ::write(fd_, bytes.data(), bytes.size());
  if (tx_bytes < 0) {
    throw std::runtime_error("Error occurred when writing to serial port");
  }
  if ((std::size_t)tx_bytes != bytes.size()) {
    throw std::runtime_error("Failed to write all the data requested");
  }
}

} // namespace dataroute
