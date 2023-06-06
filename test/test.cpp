#include "utest.h"

#include <iostream>
#include <string>
#include <array>
#include <thread>

#include "json_server.hpp"
#include "json_client.hpp"

UTEST_STATE();

using client = json_client::EndpointConnection;
using basic_type = json_client::types::BasicType;

//
// Tests for reading data
//
UTEST(Get, basic_int)
{
    const auto val = client("/basic/int").get<int64_t>();
    ASSERT_EQ(val, 3);
}

UTEST(Get, basic_bool)
{
    const auto val = client("/basic/bool").get<bool>();
    ASSERT_FALSE(val);
}

UTEST(Get, basic_float)
{
    const auto val = client("/basic/float").get<float>();
    ASSERT_NEAR(val, -24.0, 1e-6);
}

UTEST(Get, basic_string)
{
    const auto val = client("/basic/string").get<std::string>();
    ASSERT_STREQ(val.c_str(), "DEBUG");
}

UTEST(Get, vector_homogenous)
{
    const auto vec = client("/array/homogenous").get<std::vector<basic_type>>();
    ASSERT_EQ(vec.size(), 19);

    std::size_t idx = 0;
    for (int64_t expected = -9; expected < 9; ++expected, idx++)
    {
        const auto val = std::get<decltype(expected)>(vec.at(idx));
        ASSERT_EQ(val, expected);
    }
}

UTEST(Get, vector_hetereogenous)
{
    const auto vec = client("/array/heterogenous").get<std::vector<basic_type>>();
    ASSERT_EQ(vec.size(), 5);
    ASSERT_TRUE(std::get<bool>(vec.at(0)));
    ASSERT_EQ(std::get<int64_t>(vec.at(1)), 1);
    ASSERT_STREQ(std::get<std::string>(vec.at(2)).c_str(), "TEST");
    ASSERT_NEAR(std::get<float>(vec.at(3)), -10.0, 1e-6);
    ASSERT_EQ(std::get<int64_t>(vec.at(4)), -2);
}

//
// Test for writing data
//
UTEST(Set, basic_int)
{
    auto endpoint = client("/basic/int");
    const auto orig_val = endpoint.get<int64_t>();
    for (int64_t i = -10; i < 10; ++i)
    {
        endpoint.set(i);
        ASSERT_EQ(endpoint.get<int64_t>(), i);
    }
    endpoint.set(orig_val);
    ASSERT_EQ(endpoint.get<int64_t>(), orig_val);
}
// TODO: more set tests


//
// Test errors
//
UTEST(Errors, invalid_path)
{
    auto endpoint = client("/invalid");

    bool is_thrown = false;
    try
    {
        const auto _ = endpoint.get<int64_t>();
    }
    catch (const json_server::RuntimeException &e)
    {
        ASSERT_EQ(e.m_err_code, json_server::error_code::json_path_error);
        is_thrown = true;
    }
    ASSERT_TRUE(is_thrown);
}

UTEST(Errors, type_mismatch)
{
    auto endpoint = client("/basic/string");

    bool is_thrown = false;
    try
    {
        const auto _ = endpoint.get<int64_t>();
    }
    catch (const json_server::RuntimeException &e)
    {
        ASSERT_EQ(e.m_err_code, json_server::error_code::type_error);
        is_thrown = true;
    }
    ASSERT_TRUE(is_thrown);
}

//
// Lock and unlock
//
UTEST(Concurrency, lock)
{
    auto client_1 = client("/basic/int");
    client_1.lock();
    client_1.set(-1);

    auto t = std::thread(
        []()
        {
            auto client_2 = client("/basic/int", true);

            for (uint32_t i = 1; i < 101; ++i)
            {
                client_2.set(client_2.get<int64_t>() + i);
            }
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    client_1.set<int64_t>(0);
    client_1.unlock();
    // client_2 in thread should now be able to run
    t.join();

    ASSERT_EQ(client_1.get<int64_t>(), 5050);
}


int main(int argc, char **argv)
{
    json_server::init("test_data.json");
    return utest_main(argc, argv);
}
