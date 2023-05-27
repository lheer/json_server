/*
 * Common code shared between client and server implementation.
 */
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "exceptions.hpp"


namespace details
{

enum class request_cmd : uint8_t
{
    read,
    write
};

// Internal error codes for client <-> server communication
enum class error_code_int : uint8_t
{
    none,
    json_parse,
    invalid_path,
    value_error
};

const std::string_view DEFAULT_SOCK_FILE = "/tmp/json_server.sock";


// Send size information struct via some socket type (both client and server).
template <typename T>
void transmit_size_info(T &socket, const uint32_t payload_size)
{
    // Note: No need to take care of byte ordering here, since all communication is local only
    uint32_t sz = payload_size;
    auto *const buffer = reinterpret_cast<uint8_t *>(&sz);
    const auto ret = socket.write_n(buffer, sizeof(uint32_t));

    if (ret != sizeof(uint32_t))
    {
        throw err::InternalException(lh::nostd::source_location::current(), "write failed: {}, {}", socket.last_error(),
                                     socket.last_error_str());
    }
}

// Read size information from some socket type (both client and server). Returns 0 when endpoint is closed.
template <typename T>
[[nodiscard]] uint32_t receive_size_info(T &socket)
{
    std::array<uint8_t, 4> size_buffer{};
    auto ret = socket.read_n(size_buffer.begin(), sizeof(uint32_t));

    if (ret == 0)
    {
        // Peer closed connection
        return 0;
    }
    if (ret != sizeof(uint32_t))
    {
        throw err::InternalException(lh::nostd::source_location::current(), "read failed: {}, {}", socket.last_error(),
                                     socket.last_error_str());
    }

    uint32_t expected{0};
    // Note: No need to take care of byte ordering here, since all communication is local only
    std::memcpy(&expected, size_buffer.data(), sizeof(uint32_t));
    return expected;
}
} // namespace details
