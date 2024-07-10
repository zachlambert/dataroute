#include <dataroute/serial.hpp>
#include <string_view>
#include <iostream>

int main() {
    dataroute::SerialPort port;
    port.connect("/dev/ttyACM0", 115200);

    while (true) {
        auto bytes = port.read();
        std::cout << std::string_view((char*)bytes.data(), bytes.size());
    }
}
