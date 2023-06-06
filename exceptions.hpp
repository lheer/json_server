#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <cstdint>

#include <fmt/core.h>


namespace lh::nostd
{
// Basic C++20 std::source_location reimplemetation for older versions (GCC and clang only)
struct source_location
{
public:
#if not defined(__apple_build_version__) and defined(__clang__) and (__clang_major__ >= 9)
    static constexpr source_location current(const char *file_name = __builtin_FILE(),
                                             const char *function_name = __builtin_FUNCTION(),
                                             const uint_least32_t line_number = __builtin_LINE()) noexcept
#elif defined(__GNUC__) and (__GNUC__ > 4 or (__GNUC__ == 4 and __GNUC_MINOR__ >= 8))
    static constexpr source_location current(const char *file_name = __builtin_FILE(),
                                             const char *function_name = __builtin_FUNCTION(),
                                             const uint_least32_t line_number = __builtin_LINE()) noexcept
#else
#error unsupported compiler
#endif
    {
        return {file_name, function_name, line_number};
    }

    source_location(const source_location &) = default;
    source_location(source_location &&) = default;

    [[nodiscard]] constexpr const char *file_name() const noexcept
    {
        return m_file_name;
    }

    [[nodiscard]] constexpr const char *function_name() const noexcept
    {
        return m_function_name;
    }

    [[nodiscard]] constexpr uint_least32_t line() const noexcept
    {
        return m_line_number;
    }

private:
    constexpr source_location(const char *file_name, const char *function_name,
                              const uint_least32_t line_number) noexcept
        : m_file_name(file_name), m_function_name(function_name), m_line_number(line_number)
    {
    }

    const char *m_file_name;
    const char *m_function_name;
    const std::uint_least32_t m_line_number;
};
} // namespace lh::nostd


namespace json_server
{
namespace details
{
    // Exception that takes a formattable string.
    class FormattedException : public std::exception
    {
    public:
        FormattedException() = default;

        template <typename... Args>
        explicit FormattedException(const std::string_view msg, const Args &...args) : m_msg(fmt::format(msg, args...))
        {
        }

        [[nodiscard]] const char *what() const noexcept override
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg{};
    };

    // Exception that takes a source location from where it was thrown and a formattable string.
    class LocationException : public std::exception
    {
    public:
        LocationException() = default;

        template <typename... Args>
        LocationException(const lh::nostd::source_location &loc, const std::string_view msg, const Args &...args)
            : m_msg(fmt::format("{}, in {}, line {}", loc.file_name(), loc.function_name(), loc.line()) + " - " +
                    fmt::format(msg, args...))
        {
        }

        [[nodiscard]] const char *what() const noexcept override
        {
            return m_msg.c_str();
        }

    private:
        std::string m_msg{};
    };

} // namespace details

// Exception class for internal errors.
class InternalException : public details::LocationException
{
    using details::LocationException::LocationException;
};

// Error codes
enum class error_code : uint8_t
{
    none,
    file_not_found,
    json_parse_error,
    socket_error,
    type_error,
    json_path_error,
    lock
};

// Main exception class with an error code for all public API errors.
class RuntimeException : public details::FormattedException
{
public:
    template <typename... Args>
    RuntimeException(const error_code err_code, const std::string_view msg, const Args &...args)
        : details::FormattedException(msg, args...), m_err_code(err_code)
    {
    }

    error_code m_err_code;
};

} // namespace json_server
