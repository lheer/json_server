# JSON Server

A simple C++17 data server library that provides access to resources stored in a JSON file via a client API.
Also, a simple and straight-forward way to access your application data from everywhere in your code so you don't
have to worry about a proper architecture :wink:

## Description

### Overview

This library provides a small server and client library for accessing data that is stored in a JSON file.
Design goals are ease of use and a simple integration with little dependencies. This makes it ideal to use
in applications running in constrained environments, for example embedded Linux where you typically don't want
to pull in loads of dependencies for a full-blown database.

### Implementation

The server loads the given JSON file into memory and allows connections via Unix Domain Sockets. Clients use
a simple, msg-packed protocol to read and modify values on the server.

## Getting Started

Make sure that the following external dependencies can by found as CMake packages:
* [fmt](https://github.com/fmtlib/fmt)

### Usage
First, store your data in some JSON text file. Then, in your code, initialize the server:

```cpp
#include "json_server.hpp"

json_server::init("data.json");
```

Then you can retrieve and manipulate the data by using the client API:

```cpp
#include "json_client.hpp"

auto data_connection = json_client::EndpointConnection("/path/to/json/resource");
const auto read_val = data_connection.get<int64_t>();
data_connection.set(12345);
```

For a more complete overview of the provided functionality, the API tests in [test.cpp](test/test.cpp) can be used.

## TODOs and Ideas

* Thread pool on server to avoid spawning a new thread for each client connection
* Synchronize changes back to the JSON file on disk

## Version History



## License

This project is licensed under the MIT License - see the LICENSE file for details

## Acknowledgments

The following libraries are included:
* [nlohmann-json](https://github.com/nlohmann/json)
* [sockpp](https://github.com/fpagliughi/sockpp) (as submodule)
* [utest.h](https://github.com/sheredom/utest.h)