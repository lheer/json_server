#include <chrono>
#include <thread>
#include <vector>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include "json_server.hpp"
#include "json_client.hpp"


int main()
{
    json_server::init("model.json");

    {
        auto con = json_client::EndpointConnection("/video/source");
        con.set(5);

        const auto val = con.get<int64_t>();
        fmt::print("val={}\n", val);
    }

    {
        auto con = json_client::EndpointConnection("/array");
        con.set<std::vector<int64_t>>({-1, 0, 1, 2, 3, 4});

        auto val = con.get<std::vector<int64_t>>();
        fmt::print("{}\n", val);
    }

    {
        auto t = std::thread(
            []
            {
                auto con = json_client::EndpointConnection("/int");
                for (int i = 0; i < 100; ++i)
                {
                    con.set(con.get<int64_t>() + i);
                }
            });

        auto con = json_client::EndpointConnection("/int");
        con.set(5);
        t.join();

        auto val = con.get<int64_t>();
        fmt::print("val={}\n", val);
    }

    return EXIT_SUCCESS;
}
