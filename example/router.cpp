#include <dataroute/router.hpp>

#include <thread>
#include <mutex>
#include <datapack/common/primitive.hpp>
#include <datapack/util/debug.hpp>


struct Buffer {
    std::mutex mutex;
    std::vector<std::uint8_t> data;
    std::size_t head;
    std::size_t tail;
    Buffer(std::size_t size):
        data(size),
        head(0),
        tail(0)
    {}
};
Buffer buffer_a(1024);
Buffer buffer_b(1024);

class FakeInterface: public dataroute::Interface {
public:
    FakeInterface(Buffer& buffer_in, Buffer& buffer_out):
        buffer_in(buffer_in),
        buffer_out(buffer_out)
    {}

    void write(const std::span<const std::uint8_t>& bytes) override {
        auto lock = std::scoped_lock<std::mutex>(buffer_out.mutex);
        for (auto byte: bytes) {
            buffer_out.data[buffer_out.tail] = byte;
            buffer_out.tail = (buffer_out.tail + 1) % buffer_out.data.size();
            if (buffer_out.tail == buffer_out.head) {
                buffer_out.head = (buffer_out.head + 1) % buffer_out.data.size();
            }
        }
    }

    std::span<const std::uint8_t> read() override {
        auto lock = std::scoped_lock<std::mutex>(buffer_in.mutex);
        buffer_local.clear();
        while (buffer_in.head != buffer_in.tail) {
            buffer_local.push_back(buffer_in.data[buffer_in.head]);
            buffer_in.head = (buffer_in.head + 1) % buffer_in.data.size();
        }
        return buffer_local;
    }

private:
    std::vector<std::uint8_t> buffer_local;
    Buffer& buffer_in;
    Buffer& buffer_out;
};

struct Message {
    std::string message;
    int counter;
    Message():
        counter(0)
    {}

    DATAPACK_METHODS(Message);
};

template <typename Visitor>
void Message::visit(Visitor& visitor) {
    visitor.value("message", message);
    visitor.value("counter", counter);
}
DATAPACK_METHODS_IMPL(Message);

int main() {
    dataroute::Router router_a(FakeInterface(buffer_a, buffer_b));
    dataroute::Router router_b(FakeInterface(buffer_b, buffer_a));

    std::uint16_t id_a_message = 0;
    std::uint16_t id_b_message = 0;

    std::mutex cout_mutex;

    router_a.add_callback<Message>(id_b_message, [&](const Message& message) {
        auto lock = std::scoped_lock<std::mutex>(cout_mutex);
        std::cout << "B->A RECEIVE:\n" << datapack::debug(message);
    });
    router_b.add_callback<Message>(id_a_message, [&](const Message& message) {
        auto lock = std::scoped_lock<std::mutex>(cout_mutex);
        std::cout << "A->B RECEIVE:\n" << datapack::debug(message);
    });

    using clock_t = std::chrono::high_resolution_clock;

    auto thread_a = std::jthread([&]() {
        Message message;
        message.message = "hello b";
        auto next_send = clock_t::now();
        while (true) {
            if (clock_t::now() > next_send) {
                {
                    auto lock = std::scoped_lock<std::mutex>(cout_mutex);
                    std::cout << "A->B SEND:\n" << datapack::debug(message);
                }
                router_a.write(id_a_message, message);
                message.counter++;
                next_send += std::chrono::milliseconds(100);
            }
            router_a.poll();
        }
    });

    auto thread_b = std::jthread([&]() {
        Message message;
        message.message = "hello a";
        auto next_send = clock_t::now();
        while (true) {
            if (clock_t::now() > next_send) {
                {
                    auto lock = std::scoped_lock<std::mutex>(cout_mutex);
                    std::cout << "B->A SEND:\n" << datapack::debug(message);
                }
                router_b.write(id_b_message, message);
                message.counter++;
                next_send += std::chrono::milliseconds(500);
            }
            router_a.poll();
        }
    });
}
