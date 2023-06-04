#include "utest.h"

#include <iostream>
#include <string>
#include <array>

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
// Test errors
//
UTEST(Errors, invalid_path)
{
    ASSERT_EXCEPTION(client("/invalid").get<int64_t>(), json_server::RuntimeException);
}

UTEST(Errors, type_mismatch)
{
    ASSERT_EXCEPTION(client("/basic/int").get<bool>(), json_server::RuntimeException);
}


int main(int argc, char **argv)
{
    json_server::init("test_data.json");
    return utest_main(argc, argv);
}
