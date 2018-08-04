#pragma once

#include <cstddef> // size_t
#include <string> // to_string

namespace nlohmann
{
namespace detail
{
struct source_location_t
{
    std::size_t byte_pos;
    std::size_t line;
    std::size_t column;
    explicit constexpr source_location_t(
        std::size_t byte_pos = 0,
        std::size_t line = 0,
        std::size_t column = 0
    ) noexcept
    : byte_pos{byte_pos}
    , line{line}
    , column{column}
    {}
};
}
}

namespace std
{
    inline std::string to_string(nlohmann::detail::source_location_t loc)
    {
        return
            "byte: " + to_string(loc.byte_pos) +
            " line: " + to_string(loc.line) +
            " column: " + to_string(loc.column);
    }
}