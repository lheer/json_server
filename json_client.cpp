#include "json_client.hpp"

#include "nlohmann/json.hpp"
#include "exceptions.hpp"


namespace json_client
{

namespace impl
{
    types::BasicType json_to_basic(const nlohmann::json &j_val)
    {
        if (!j_val.is_primitive())
        {
            throw json_server::InternalException(lh::nostd::source_location::current(), "Not a primitive type");
        }
        if (j_val.is_number_integer())
        {
            return {j_val.get<int64_t>()};
        }
        else if (j_val.is_boolean())
        {
            return {j_val.get<bool>()};
        }
        else if (j_val.is_number_float())
        {
            return {j_val.get<float>()};
        }
        else if (j_val.is_string())
        {
            return {j_val.get<std::string>()};
        }
        else
        {
            throw json_server::InternalException(lh::nostd::source_location::current(), "Invalid type: {}",
                                                 j_val.type_name());
        }
    }

    nlohmann::json basic_to_json(const types::BasicType &val)
    {
        if (std::holds_alternative<bool>(val))
        {
            nlohmann::json ret(nlohmann::json::value_t::boolean);
            ret = std::get<bool>(val);
            return ret;
        }
        else if (std::holds_alternative<int64_t>(val))
        {
            nlohmann::json ret(nlohmann::json::value_t::number_integer);
            ret = std::get<int64_t>(val);
            return ret;
        }
        else if (std::holds_alternative<float>(val))
        {
            nlohmann::json ret(nlohmann::json::value_t::number_float);
            ret = std::get<float>(val);
            return ret;
        }
        else if (std::holds_alternative<std::string>(val))
        {
            nlohmann::json ret(nlohmann::json::value_t::string);
            ret = std::get<std::string>(val);
            return ret;
        }
        else
        {
            throw json_server::InternalException(lh::nostd::source_location::current(), "Invalid type: {}",
                                                 val.index());
        }
    }
} // namespace impl

EndpointConnection::EndpointConnection(const std::string &resource_path, const std::filesystem::path &socket_file)
    : m_resource_path(resource_path), m_socket_file(socket_file)
{
    if (!m_srv_con.connect(sockpp::unix_address(m_socket_file)))
    {
        throw json_server::RuntimeException(json_server::error_code::socket_error,
                                            "Unable to connect to socket file {}", m_socket_file.string());
    }
    m_resource_path = resource_path;
}

std::tuple<json_server::error_code, nlohmann::json> EndpointConnection::read_server_reply()
{
    const auto sz = details::receive_size_info(m_srv_con);
    std::vector<uint8_t> buffer;
    buffer.resize(sz);

    m_srv_con.read_n(buffer.data(), buffer.size());
    nlohmann::json j_obj;
    j_obj = nlohmann::json::from_msgpack(buffer);
    return {static_cast<json_server::error_code>(j_obj.at("err_code").get<int>()), j_obj.at("value")};
}

nlohmann::json EndpointConnection::get_impl()
{
    nlohmann::json req;
    req["cmd"] = static_cast<uint8_t>(details::request_cmd::read);
    req["path"] = m_resource_path;

    // Transmit size and payload
    const auto as_msgpack = nlohmann::json::to_msgpack(req);
    details::transmit_size_info(m_srv_con, static_cast<uint32_t>(as_msgpack.size()));

    const auto ret = m_srv_con.write_n(as_msgpack.data(), as_msgpack.size());
    if (ret == -1)
    {
        throw json_server::InternalException(lh::nostd::source_location::current(), "Write failed with code {}, msg {}",
                                             m_srv_con.last_error(), m_srv_con.last_error_str());
    }
    if (static_cast<size_t>(ret) != as_msgpack.size())
    {
        throw json_server::InternalException(lh::nostd::source_location::current(),
                                             "Write failed: not all bytes transmitted", ret);
    }

    // Receive answer
    const auto [err, j_val] = read_server_reply();
    if (err != json_server::error_code::none)
    {
        throw json_server::RuntimeException(err, "get failed");
    }
    return j_val;
}

void EndpointConnection::set_impl(const nlohmann::json &val)
{
    // Construct request
    nlohmann::json req;
    req["cmd"] = static_cast<uint8_t>(details::request_cmd::write);
    req["path"] = m_resource_path;
    req["value"] = val;

    // Send size and payload
    const auto as_msgpack = nlohmann::json::to_msgpack(req);
    details::transmit_size_info(m_srv_con, static_cast<uint32_t>(as_msgpack.size()));

    const auto ret = m_srv_con.write_n(as_msgpack.data(), as_msgpack.size());
    if (ret == -1)
    {
        throw json_server::InternalException(lh::nostd::source_location::current(), "Write failed with code {}", ret);
    }
    if (static_cast<size_t>(ret) != as_msgpack.size())
    {
        throw json_server::InternalException(lh::nostd::source_location::current(),
                                             "Write failed: not all bytes transmitted", ret);
    }

    // Receive answer
    const auto [err, _] = read_server_reply();
    if (err != json_server::error_code::none)
    {
        throw json_server::RuntimeException(err, "set failed");
    }
}

types::BasicType EndpointConnection::get_impl_basic()
{
    return impl::json_to_basic(get_impl());
}

types::CompoundType EndpointConnection::get_impl_array()
{
    const auto j_val = get_impl();

    if (!j_val.is_array())
    {
        throw json_server::InternalException(lh::nostd::source_location::current(), "Not an array type");
    }
    if (j_val.empty())
    {
        return {};
    }

    types::CompoundType ret;
    ret.resize(j_val.size());
    std::transform(j_val.begin(), j_val.end(), ret.begin(), impl::json_to_basic);
    return ret;
}

void EndpointConnection::set_impl_basic(const types::BasicType &val)
{
    set_impl(impl::basic_to_json(val));
}

void EndpointConnection::set_impl_array(const types::CompoundType &val_array)
{
    nlohmann::json set_val;
    for (const auto &val: val_array)
    {
        set_val.push_back(impl::basic_to_json(val));
    }
    set_impl(set_val);
}

EndpointConnection::~EndpointConnection()
{
    if (m_srv_con.is_open())
    {
        m_srv_con.close();
    }
}

} // namespace json_client
