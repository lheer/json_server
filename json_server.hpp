#pragma once

#include <filesystem>

#include "details.hpp"


namespace json_server
{

// Initializes the json model with a json file as resource backend. Starts a server to which clients can connect.
void init(const std::filesystem::path &json_resource,
          const std::filesystem::path &socket_file = details::DEFAULT_SOCK_FILE);
} // namespace json_server
