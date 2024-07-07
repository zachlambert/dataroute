#include <dataroute/serial.hpp>
#include <string_view>
#include <iostream>

int main() {
    dataroute::SerialPort port;
    port.connect("/dev/ttyACM0", 115200);

    while (true) {
        auto bytes = port.read(1024);
        std::cout << std::string_view((char*)bytes.data(), bytes.size());
    }
}
