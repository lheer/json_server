#pragma once

#include <filesystem>
#include <variant>
#include <string>
#include <cstdint>
#include <algorithm>

#include "details.hpp"

#include "nlohmann/json_fwd.hpp"
#include "sockpp/unix_connector.h"


namespace json_client
{

// We use our own type system to avoid including json.hpp every time someone uses this client header.
// This reduces compile-times by quite a bit but sacrifices performance.
namespace types
{
    // A basic type for possible JSON types
    using BasicType = std::variant<bool, int64_t, float, std::string>;
    // A vector of possibly heterogenous JSON types
    using CompoundType = std::vector<BasicType>;
} // namespace types

namespace impl
{
    // Simple type trait to detect if type is some std::vector.
    template <typename>
    struct is_std_vector : std::false_type
    {
    };

    template <typename T, typename A>
    struct is_std_vector<std::vector<T, A>> : std::true_type
    {
    };

} // namespace impl

// A connection to the model server.
class EndpointConnection
{
public:
    explicit EndpointConnection(const std::string &resource_path,
                                const std::filesystem::path &socket_file = details::DEFAULT_SOCK_FILE);
    EndpointConnection(const EndpointConnection &) = delete;
    EndpointConnection(EndpointConnection &&) = default;
    EndpointConnection &operator=(const EndpointConnection &) = delete;
    EndpointConnection &operator=(EndpointConnection &&) = default;
    ~EndpointConnection();

    // Retrieve some value from the model.
    template <typename T>
    T get()
    {
        if constexpr (impl::is_std_vector<T>::value)
        {
            // If we need to return a vector ...
            using vec_t = typename T::value_type;
            if constexpr (std::is_same_v<vec_t, types::BasicType>)
            {
                // ... of heterogenous types
                return get_impl_array();
            }
            else
            {
                // ... of homogenous types
                const auto j_vec = get_impl_array();

                std::vector<vec_t> ret;
                ret.resize(j_vec.size());
                std::transform(j_vec.begin(), j_vec.end(), ret.begin(),
                               [](const types::BasicType &v) { return std::get<vec_t>(v); });
                return ret;
            }
        }
        else
        {
            return std::get<T>(get_impl_basic());
        }
    }

    // Set some value in the model.
    template <typename T>
    void set(const T val)
    {
        if constexpr (impl::is_std_vector<T>::value)
        {
            using vec_t = typename T::value_type;
            if constexpr (std::is_same_v<vec_t, types::BasicType>)
            {
                // Vector of heterogenous types
                set_impl_array(val);
            }
            else
            {
                // Vector of homogenous types
                types::CompoundType j_vec;
                j_vec.resize(val.size());
                std::transform(val.begin(), val.end(), j_vec.begin(),
                               [](const auto &v) { return types::BasicType{v}; });
                set_impl_array(j_vec);
            }
        }
        else
        {
            set_impl_basic({val});
        }
    }


private:
    std::string m_resource_path;
    sockpp::unix_connector m_srv_con;
    std::filesystem::path m_socket_file;

    // Read server response object consisting of an error code and some value.
    std::tuple<details::error_code_int, nlohmann::json> read_server_reply();

    // Get some JSON value from the server (implementation for nlohmann::json type).
    nlohmann::json get_impl();
    // Get some JSON value as basic type from the server.
    types::BasicType get_impl_basic();
    // Get some JSON array valuefrom the server.
    types::CompoundType get_impl_array();

    // Set some JSON value on the server (implementation for nlohmann::json type).
    void set_impl(const nlohmann::json &val);
    // Set some basic JSON value on the server.
    void set_impl_basic(const types::BasicType &val);
    // Set some JSON array value on the server.
    void set_impl_array(const types::CompoundType &val_array);
};


} // namespace json_client
