# JSON Server

A simple C++17 data server that provides access to resources stored in a JSON file via a client API.

## Description

An in-depth paragraph about your project and overview of use.

## Getting Started

### Dependencies

* Describe any prerequisites, libraries, OS version, etc., needed before installing program.
* ex. Windows 10

### Usage

* How/where to download your program
* Any modifications needed to be made to files/folders

## TODOs

* Build as a library, provide proper CMake integration
* Implement a lock mechanism for exclusive access to a server resource (e.g. EndpointConnection::lock and ::unlock, preferrably via RAII)
* Thread pool on server to avoid spawning a new thread for each client connection
* Implement tests

## Version History

* 0.1.0
    * Initial Release

## License

This project is licensed under the MIT License - see the LICENSE file for details

## Acknowledgments

The following libraries are included:
* [nlohmann-json](https://github.com/nlohmann/json)
* [spdlog](https://github.com/gabime/spdlog)

The following libraries are required, but not included:
* [fmt](https://github.com/fmtlib/fmt)
