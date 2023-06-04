# JSON Server

A simple C++17 data server that provides access to resources stored in a JSON file via a client API.

## Description

An in-depth paragraph about your project and overview of use.

## Getting Started

### Dependencies

* [fmt](https://github.com/fmtlib/fmt)

### Usage

* How/where to download your program
* Any modifications needed to be made to files/folders

## TODOs

* Implement a lock mechanism for exclusive access to a server resource (e.g. EndpointConnection::lock and ::unlock, preferrably via RAII)
* Thread pool on server to avoid spawning a new thread for each client connection

## Version History

* 0.1.0
    * Initial Release

## License

This project is licensed under the MIT License - see the LICENSE file for details

## Acknowledgments

The following libraries are included:
* [nlohmann-json](https://github.com/nlohmann/json)
* [spdlog](https://github.com/gabime/spdlog)
