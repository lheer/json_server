#include "json_server.hpp"

#include <map>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>


#include "nlohmann/json.hpp"
#include "sockpp/unix_acceptor.h"

#include "exceptions.hpp"
#include "details.hpp"


using json = nlohmann::json;


namespace json_server
{

namespace
{
    sockpp::unix_acceptor g_srv_acceptor{};
    std::mutex g_model_mutex{};
    json g_model{};
    std::filesystem::path g_uds_socket_file{};
    std::map<std::string, std::mutex> g_mutex_map{};

    // Accept incoming client connections and dispatch them to the handler function f_callback.
    void server_loop(std::function<void(sockpp::unix_socket sock)> f_callback)
    {
        while (true)
        {
            auto sock = g_srv_acceptor.accept();
            std::thread thr(f_callback, std::move(sock));
            thr.detach();
        }
    }

    // Reply calls by sending the value back to the client with optional error.
    void transmit_server_reply(sockpp::unix_socket &socket, const json &val, const ::json_server::error_code &err)
    {
        json j_reply;
        j_reply["value"] = val;
        j_reply["err_code"] = static_cast<int>(err);

        const auto as_msgpack = json::to_msgpack(j_reply);

        // Send size
        ::details::transmit_size_info(socket, static_cast<uint32_t>(as_msgpack.size()));

        // Send payload
        const auto ret = socket.write_n(as_msgpack.data(), as_msgpack.size());
        if (ret < 0)
        {
            throw json_server::InternalException(lh::nostd::source_location::current(),
                                                 "send_error: Unable to send status response: socket error");
        }
        if (static_cast<size_t>(ret) != as_msgpack.size())
        {
            throw json_server::InternalException(
                lh::nostd::source_location::current(),
                "send_error: Unable to send status response: Not all bytes transmitted");
        }
    }

    // Handle a client connection.
    void client_handler(sockpp::unix_socket socket)
    {
        while (true)
        {
            // Read the size of the payload to receive
            const auto expected = ::details::receive_size_info(socket);
            if (expected == 0)
            {
                // Client closed connection
                socket.close();
                break;
            }

            // Read the actual payload
            std::vector<uint8_t> payload_buffer;
            payload_buffer.resize(expected);
            auto ret = socket.read_n(payload_buffer.data(), expected);

            if (ret < 0)
            {
                socket.close();
                throw json_server::InternalException(lh::nostd::source_location::current(),
                                                     "client_handler: Unable to read from socket: {}, {}",
                                                     socket.last_error(), socket.last_error_str());
            }

            if (ret != expected)
            {
                socket.close();
                throw json_server::InternalException(lh::nostd::source_location::current(),
                                                     "client_handler: Got {} bytes, expected {} bytes", ret, expected);
            }

            // Convert received payload to to json object and react upon the request
            auto j_recv = json::from_msgpack(payload_buffer);
            try
            {
                const auto cmd_code = static_cast<::details::request_cmd>(j_recv.at("cmd").get<int>());
                const auto path = j_recv.at("path").get<std::string>();

                switch (cmd_code)
                {
                    case ::details::request_cmd::read:
                    {
                        // Read some value on the model
                        json val;
                        {
                            const std::scoped_lock lock(g_model_mutex);
                            val = g_model.at(nlohmann::json_pointer<std::string>(path));
                        }
                        transmit_server_reply(socket, val, ::json_server::error_code::none);
                        break;
                    }
                    case ::details::request_cmd::write:
                    {
                        // Update value in json model
                        {
                            const std::scoped_lock lock(g_model_mutex);
                            g_model.at(nlohmann::json_pointer<std::string>(path)) = j_recv.at("value");
                        }
                        transmit_server_reply(socket, json::value_t::null, ::json_server::error_code::none);
                        break;
                    }
                    case ::details::request_cmd::lock:
                    {
                        g_mutex_map[path].lock();
                        transmit_server_reply(socket, json::value_t::null, ::json_server::error_code::none);
                        break;
                    }
                    case ::details::request_cmd::unlock:
                    {
                        g_mutex_map.at(path).unlock();
                        transmit_server_reply(socket, json::value_t::null, ::json_server::error_code::none);
                        break;
                    }
                }
            }
            catch (const json::out_of_range &e)
            {
                // Got client request with invalid json path: Send error and abort connection
                transmit_server_reply(socket, json::value_t{0}, ::json_server::error_code::json_path_error);
                socket.close();
                return;
            }
        }
    }

} // namespace

void init(const std::filesystem::path &json_resource, const std::filesystem::path &socket_file)
{
    if (!std::filesystem::is_regular_file(json_resource))
    {
        throw json_server::RuntimeException(json_server::error_code::file_not_found, "No such file: {}",
                                            json_resource.string());
    }

    try
    {
        std::ifstream fs(json_resource);
        g_model = json::parse(fs);
    }
    catch (const json::parse_error &e)
    {
        throw json_server::RuntimeException(json_server::error_code::json_parse_error, "JSON parse error: {}",
                                            e.what());
    }

    sockpp::initialize();

    if (std::filesystem::is_socket(socket_file))
    {
        std::filesystem::remove(socket_file);
    }
    if (!g_srv_acceptor.open(sockpp::unix_address(socket_file)))
    {
        throw json_server::RuntimeException(json_server::error_code::socket_error, "Unable to open unix socket {}",
                                            socket_file.string());
    }
    g_uds_socket_file = socket_file;

    auto thr = std::thread(server_loop, client_handler);
    thr.detach();
}

} // namespace json_server
