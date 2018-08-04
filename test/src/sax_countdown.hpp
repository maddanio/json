#pragma once

#include <nlohmann/json.hpp>

class SaxCountdown : public nlohmann::json::json_sax_t
{
  public:
    explicit SaxCountdown(const int count) : events_left(count)
    {}

    bool null(source_location_t) override
    {
        return events_left-- > 0;
    }

    bool boolean(bool, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool number_integer(nlohmann::json::number_integer_t, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool number_unsigned(nlohmann::json::number_unsigned_t, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool number_float(nlohmann::json::number_float_t, const std::string&, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool string(std::string&, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool start_object(std::size_t, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool key(std::string&, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool end_object(source_location_t) override
    {
        return events_left-- > 0;
    }

    bool start_array(std::size_t, source_location_t) override
    {
        return events_left-- > 0;
    }

    bool end_array(source_location_t) override
    {
        return events_left-- > 0;
    }

    bool parse_error(const std::string&, const nlohmann::json::exception&) override
    {
        return false;
    }

  private:
    int events_left = 0;
};