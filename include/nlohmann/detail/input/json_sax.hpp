#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <nlohmann/detail/input/parser.hpp>
#include <nlohmann/detail/exceptions.hpp>

namespace nlohmann
{

/*!
@brief SAX interface
*/
template<typename BasicJsonType>
struct json_sax
{
    /// type for (signed) integers
    using number_integer_t = typename BasicJsonType::number_integer_t;
    /// type for unsigned integers
    using number_unsigned_t = typename BasicJsonType::number_unsigned_t;
    /// type for floating-point numbers
    using number_float_t = typename BasicJsonType::number_float_t;
    /// type for strings
    using string_t = typename BasicJsonType::string_t;
    using source_location_t = typename nlohmann::detail::source_location_t;

    /// constant to indicate that no size limit is given for array or object
    static constexpr auto no_limit = std::size_t(-1);

    virtual bool null(source_location_t loc)
    {
        return null();
    }
    virtual bool boolean(bool val, source_location_t loc)
    {
        return boolean(val);
    }
    virtual bool number_integer(number_integer_t val, source_location_t loc)
    {
        return number_integer(val);
    }
    virtual bool number_unsigned(number_unsigned_t val, source_location_t loc)
    {
        return number_unsigned(val);
    }
    virtual bool number_float(number_float_t val, const string_t& s, source_location_t loc)
    {
        return number_float(val, s);
    }
    virtual bool string(string_t& val, source_location_t loc)
    {
        return string(val);
    }
    virtual bool start_object(std::size_t elements, source_location_t loc)
    {
        return start_object(elements);
    }
    virtual bool key(string_t& val, source_location_t loc)
    {
        return key(val);
    }
    virtual bool end_object(source_location_t loc)
    {
        return end_object();
    }
    virtual bool start_array(std::size_t elements, source_location_t loc)
    {
        return start_array(elements);
    }
    virtual bool end_array(source_location_t loc)
    {
        return end_array();
    }
    /*!
    @brief a null value was read
    @return whether parsing should proceed
    */
    virtual bool null()
    {
        return true;
    };

    /*!
    @brief a boolean value was read
    @param[in] val  boolean value
    @return whether parsing should proceed
    */
    virtual bool boolean(bool val)
    {
        return true;
    };

    /*!
    @brief an integer number was read
    @param[in] val  integer value
    @return whether parsing should proceed
    */
    virtual bool number_integer(number_integer_t val)
    {
        return true;
    };

    /*!
    @brief an unsigned integer number was read
    @param[in] val  unsigned integer value
    @return whether parsing should proceed
    */
    virtual bool number_unsigned(number_unsigned_t val)
    {
        return true;
    };

    /*!
    @brief an floating-point number was read
    @param[in] val  floating-point value
    @param[in] s    raw token value
    @return whether parsing should proceed
    */
    virtual bool number_float(number_float_t val, const string_t& s)
    {
        return true;
    };

    /*!
    @brief a string was read
    @param[in] val  string value
    @return whether parsing should proceed
    */
    virtual bool string(string_t& val)
    {
        return true;
    };

    /*!
    @brief the beginning of an object was read
    @param[in] elements  number of object elements or no_limit if unknown
    @return whether parsing should proceed
    @note binary formats may report the number of elements
    */
    virtual bool start_object(std::size_t elements)
    {
        return true;
    };

    /*!
    @brief an object key was read
    @param[in] val  object key
    @return whether parsing should proceed
    */
    virtual bool key(string_t& val)
    {
        return true;
    };

    /*!
    @brief the end of an object was read
    @return whether parsing should proceed
    */
    virtual bool end_object()
    {
        return true;
    };

    /*!
    @brief the beginning of an array was read
    @param[in] elements  number of array elements or no_limit if unknown
    @return whether parsing should proceed
    @note binary formats may report the number of elements
    */
    virtual bool start_array(std::size_t elements)
    {
        return true;
    };

    /*!
    @brief the end of an array was read
    @return whether parsing should proceed
    */
    virtual bool end_array()
    {
        return true;
    };

    /*!
    @brief a parse error occurred
    @param[in] position    the position in the input where the error occurs
    @param[in] last_token  the last read token
    @param[in] error_msg   a detailed error message
    @return whether parsing should proceed (must return false)
    */
    virtual bool parse_error(
        const std::string& last_token,
        const detail::exception& ex
    )
    {
        return false;
    };

    virtual ~json_sax() = default;
};


namespace detail
{
/*!
@brief SAX implementation to create a JSON value from SAX events

This class implements the @ref json_sax interface and processes the SAX events
to create a JSON value which makes it basically a DOM parser. The structure or
hierarchy of the JSON value is managed by the stack `ref_stack` which contains
a pointer to the respective array or object for each recursion depth.

After successful parsing, the value that is passed by reference to the
constructor contains the parsed value.

@tparam BasicJsonType  the JSON type
*/
template<typename BasicJsonType>
class json_sax_dom_parser : public json_sax<BasicJsonType>
{
  public:
    using number_integer_t = typename BasicJsonType::number_integer_t;
    using number_unsigned_t = typename BasicJsonType::number_unsigned_t;
    using number_float_t = typename BasicJsonType::number_float_t;
    using string_t = typename BasicJsonType::string_t;
    typedef nlohmann::detail::source_location_t source_location_t;

    /*!
    @param[in, out] r  reference to a JSON value that is manipulated while
                       parsing
    @param[in] allow_exceptions_  whether parse errors yield exceptions
    */
    json_sax_dom_parser(BasicJsonType& r, const bool allow_exceptions_ = true)
        : root(r), allow_exceptions(allow_exceptions_)
    {}

    bool null(source_location_t loc) override
    {
        handle_value(nullptr, loc);
        return true;
    }

    bool boolean(bool val, source_location_t loc) override
    {
        handle_value(val, loc);
        return true;
    }

    bool number_integer(number_integer_t val, source_location_t loc) override
    {
        handle_value(val, loc);
        return true;
    }

    bool number_unsigned(number_unsigned_t val, source_location_t loc) override
    {
        handle_value(val, loc);
        return true;
    }

    bool number_float(number_float_t val, const string_t&, source_location_t loc) override
    {
        handle_value(val, loc);
        return true;
    }

    bool string(string_t& val, source_location_t loc) override
    {
        handle_value(val, loc);
        return true;
    }

    bool start_object(std::size_t len, source_location_t loc) override
    {
        ref_stack.push_back(handle_value(BasicJsonType::value_t::object, loc));

        if (JSON_UNLIKELY(len != json_sax<BasicJsonType>::no_limit and len > ref_stack.back()->max_size()))
        {
            JSON_THROW(out_of_range::create(
                           408,
                           "excessive object size: " + std::to_string(len),
                           loc
                       ));
        }

        return true;
    }

    bool key(string_t& val, source_location_t loc) override
    {
        // add null at given key and store the reference for later
        object_element = &(ref_stack.back()->m_value.object->operator[](val));
        return true;
    }

    bool end_object(source_location_t loc) override
    {
        ref_stack.pop_back();
        return true;
    }

    bool start_array(std::size_t len, source_location_t loc) override
    {
        ref_stack.push_back(handle_value(BasicJsonType::value_t::array, loc));

        if (JSON_UNLIKELY(len != json_sax<BasicJsonType>::no_limit and len > ref_stack.back()->max_size()))
        {
            JSON_THROW(out_of_range::create(
                           408,
                           "excessive array size: " + std::to_string(len),
                           loc
                       ));
        }

        return true;
    }

    bool end_array(source_location_t loc) override
    {
        ref_stack.pop_back();
        return true;
    }

    bool parse_error(
        const std::string&,
        const detail::exception& ex
    ) override
    {
        errored = true;
        if (allow_exceptions)
        {
            // determine the proper exception type from the id
            switch ((ex.id / 100) % 100)
            {
                case 1:
                    JSON_THROW(*reinterpret_cast<const detail::parse_error*>(&ex));
                case 2:
                    JSON_THROW(*reinterpret_cast<const detail::invalid_iterator*>(&ex));  // LCOV_EXCL_LINE
                case 3:
                    JSON_THROW(*reinterpret_cast<const detail::type_error*>(&ex));  // LCOV_EXCL_LINE
                case 4:
                    JSON_THROW(*reinterpret_cast<const detail::out_of_range*>(&ex));
                case 5:
                    JSON_THROW(*reinterpret_cast<const detail::other_error*>(&ex));  // LCOV_EXCL_LINE
                default:
                    assert(false);  // LCOV_EXCL_LINE
            }
        }
        return false;
    }

    constexpr bool is_errored() const
    {
        return errored;
    }

  private:
    /*!
    @invariant If the ref stack is empty, then the passed value will be the new
               root.
    @invariant If the ref stack contains a value, then it is an array or an
               object to which we can add elements
    */
    template<typename Value>
    BasicJsonType* handle_value(Value&& v, source_location_t loc)
    {
        if (ref_stack.empty())
        {
            root = BasicJsonType(std::forward<Value>(v));
            return &root;
        }
        else
        {
            assert(ref_stack.back()->is_array() or ref_stack.back()->is_object());
            if (ref_stack.back()->is_array())
            {
                ref_stack.back()->m_value.array->emplace_back(std::forward<Value>(v));
                ref_stack.back()->m_value.array->back().set_source_location(loc);
                return &(ref_stack.back()->m_value.array->back());
            }
            else
            {
                assert(object_element);
                *object_element = BasicJsonType(std::forward<Value>(v));
                object_element->set_source_location(loc);
                return object_element;
            }
        }
    }

    /// the parsed JSON value
    BasicJsonType& root;
    /// stack to model hierarchy of values
    std::vector<BasicJsonType*> ref_stack;
    /// helper to hold the reference for the next object element
    BasicJsonType* object_element = nullptr;
    /// whether a syntax error occurred
    bool errored = false;
    /// whether to throw exceptions in case of errors
    const bool allow_exceptions = true;
};

template<typename BasicJsonType>
class json_sax_dom_callback_parser : public json_sax<BasicJsonType>
{
  public:
    using number_integer_t = typename BasicJsonType::number_integer_t;
    using number_unsigned_t = typename BasicJsonType::number_unsigned_t;
    using number_float_t = typename BasicJsonType::number_float_t;
    using string_t = typename BasicJsonType::string_t;
    using parser_callback_t = typename BasicJsonType::parser_callback_t;
    using parse_event_t = typename BasicJsonType::parse_event_t;

    json_sax_dom_callback_parser(BasicJsonType& r,
                                 const parser_callback_t cb,
                                 const bool allow_exceptions_ = true)
        : root(r), callback(cb), allow_exceptions(allow_exceptions_)
    {
        keep_stack.push_back(true);
    }

    bool null(source_location_t loc) override
    {
        handle_value(nullptr, false, loc);
        return true;
    }

    bool boolean(bool val, source_location_t loc) override
    {
        handle_value(val, false, loc);
        return true;
    }

    bool number_integer(number_integer_t val, source_location_t loc) override
    {
        handle_value(val, false, loc);
        return true;
    }

    bool number_unsigned(number_unsigned_t val, source_location_t loc) override
    {
        handle_value(val, false, loc);
        return true;
    }

    bool number_float(number_float_t val, const string_t&, source_location_t loc) override
    {
        handle_value(val, false, loc);
        return true;
    }

    bool string(string_t& val, source_location_t loc) override
    {
        handle_value(val, false, loc);
        return true;
    }

    bool start_object(std::size_t len, source_location_t loc) override
    {
        // check callback for object start
        const bool keep = callback(static_cast<int>(ref_stack.size()), parse_event_t::object_start, discarded);
        keep_stack.push_back(keep);

        auto val = handle_value(BasicJsonType::value_t::object, true, loc);
        ref_stack.push_back(val.second);

        // check object limit
        if (ref_stack.back())
        {
            if (JSON_UNLIKELY(len != json_sax<BasicJsonType>::no_limit and len > ref_stack.back()->max_size()))
            {
                JSON_THROW(out_of_range::create(
                               408,
                               "excessive object size: " + std::to_string(len),
                               loc
                           ));
            }
        }

        return true;
    }

    bool key(string_t& val, source_location_t loc) override
    {
        BasicJsonType k = BasicJsonType(val);

        // check callback for key
        const bool keep = callback(static_cast<int>(ref_stack.size()), parse_event_t::key, k);
        key_keep_stack.push_back(keep);

        // add discarded value at given key and store the reference for later
        if (keep and ref_stack.back())
        {
            object_element = &(ref_stack.back()->m_value.object->operator[](val) = discarded);
        }

        return true;
    }

    bool end_object(source_location_t loc) override
    {
        if (ref_stack.back())
        {
            if (not callback(static_cast<int>(ref_stack.size()) - 1, parse_event_t::object_end, *ref_stack.back()))
            {
                // discard object
                *ref_stack.back() = discarded;
            }
        }

        assert(not ref_stack.empty());
        assert(not keep_stack.empty());
        ref_stack.pop_back();
        keep_stack.pop_back();

        if (not ref_stack.empty() and ref_stack.back())
        {
            // remove discarded value
            if (ref_stack.back()->is_object())
            {
                for (auto it = ref_stack.back()->begin(); it != ref_stack.back()->end(); ++it)
                {
                    if (it->is_discarded())
                    {
                        ref_stack.back()->erase(it);
                        break;
                    }
                }
            }
        }

        return true;
    }

    bool start_array(std::size_t len, source_location_t loc) override
    {
        const bool keep = callback(static_cast<int>(ref_stack.size()), parse_event_t::array_start, discarded);
        keep_stack.push_back(keep);

        auto val = handle_value(BasicJsonType::value_t::array, true, loc);
        ref_stack.push_back(val.second);

        // check array limit
        if (ref_stack.back())
        {
            if (JSON_UNLIKELY(len != json_sax<BasicJsonType>::no_limit and len > ref_stack.back()->max_size()))
            {
                JSON_THROW(out_of_range::create(
                               408,
                               "excessive array size: " + std::to_string(len),
                               loc
                           ));
            }
        }

        return true;
    }

    bool end_array(source_location_t loc) override
    {
        bool keep = true;

        if (ref_stack.back())
        {
            keep = callback(static_cast<int>(ref_stack.size()) - 1, parse_event_t::array_end, *ref_stack.back());
            if (not keep)
            {
                // discard array
                *ref_stack.back() = discarded;
            }
        }

        assert(not ref_stack.empty());
        assert(not keep_stack.empty());
        ref_stack.pop_back();
        keep_stack.pop_back();

        // remove discarded value
        if (not keep and not ref_stack.empty())
        {
            if (ref_stack.back()->is_array())
            {
                ref_stack.back()->m_value.array->pop_back();
            }
        }

        return true;
    }

    bool parse_error(
        const std::string&,
        const detail::exception& ex
    ) override
    {
        errored = true;
        if (allow_exceptions)
        {
            // determine the proper exception type from the id
            switch ((ex.id / 100) % 100)
            {
                case 1:
                    JSON_THROW(*reinterpret_cast<const detail::parse_error*>(&ex));
                case 2:
                    JSON_THROW(*reinterpret_cast<const detail::invalid_iterator*>(&ex));  // LCOV_EXCL_LINE
                case 3:
                    JSON_THROW(*reinterpret_cast<const detail::type_error*>(&ex));  // LCOV_EXCL_LINE
                case 4:
                    JSON_THROW(*reinterpret_cast<const detail::out_of_range*>(&ex));
                case 5:
                    JSON_THROW(*reinterpret_cast<const detail::other_error*>(&ex));  // LCOV_EXCL_LINE
                default:
                    assert(false);  // LCOV_EXCL_LINE
            }
        }
        return false;
    }

    constexpr bool is_errored() const
    {
        return errored;
    }

  private:
    /*!
    @param[in] v  value to add to the JSON value we build during parsing
    @param[in] skip_callback  whether we should skip calling the callback
               function; this is required after start_array() and
               start_object() SAX events, because otherwise we would call the
               callback function with an empty array or object, respectively.

    @invariant If the ref stack is empty, then the passed value will be the new
               root.
    @invariant If the ref stack contains a value, then it is an array or an
               object to which we can add elements

    @return pair of boolean (whether value should be kept) and pointer (to the
            passed value in the ref_stack hierarchy; nullptr if not kept)
    */
    template<typename Value>
    std::pair<bool, BasicJsonType*> handle_value(
        Value&& v,
        const bool skip_callback = false,
        source_location_t loc = source_location_t{}
    )
    {
        assert(not keep_stack.empty());

        // do not handle this value if we know it would be added to a discarded
        // container
        if (not keep_stack.back())
        {
            return {false, nullptr};
        }

        // create value
        auto value = BasicJsonType(std::forward<Value>(v));
        value.set_source_location(loc);

        // check callback
        const bool keep = skip_callback or callback(
                              static_cast<int>(ref_stack.size()),
                              parse_event_t::value,
                              value
                          );

        // do not handle this value if we just learnt it shall be discarded
        if (not keep)
        {
            return {false, nullptr};
        }

        if (ref_stack.empty())
        {
            root = std::move(value);
            return {true, &root};
        }
        else
        {
            assert(ref_stack.back()->is_array() or ref_stack.back()->is_object());
            if (ref_stack.back()->is_array())
            {
                ref_stack.back()->m_value.array->push_back(std::move(value));
                return {true, &(ref_stack.back()->m_value.array->back())};
            }
            else
            {
                // check if we should store an element for the current key
                assert(not key_keep_stack.empty());
                const bool store_element = key_keep_stack.back();
                key_keep_stack.pop_back();

                if (not store_element)
                {
                    return {false, nullptr};
                }

                assert(object_element);
                *object_element = std::move(value);
                return {true, object_element};
            }
        }
    }

    /// the parsed JSON value
    BasicJsonType& root;
    /// stack to model hierarchy of values
    std::vector<BasicJsonType*> ref_stack;
    /// stack to manage which values to keep
    std::vector<bool> keep_stack;
    /// stack to manage which object keys to keep
    std::vector<bool> key_keep_stack;
    /// helper to hold the reference for the next object element
    BasicJsonType* object_element = nullptr;
    /// whether a syntax error occurred
    bool errored = false;
    /// callback function
    const parser_callback_t callback = nullptr;
    /// whether to throw exceptions in case of errors
    const bool allow_exceptions = true;
    /// a discarded value for the callback
    BasicJsonType discarded = BasicJsonType::value_t::discarded;
};

template<typename BasicJsonType>
class json_sax_acceptor : public json_sax<BasicJsonType>
{
  public:
    using number_integer_t = typename BasicJsonType::number_integer_t;
    using number_unsigned_t = typename BasicJsonType::number_unsigned_t;
    using number_float_t = typename BasicJsonType::number_float_t;
    using string_t = typename BasicJsonType::string_t;

    bool null() override
    {
        return true;
    }

    bool boolean(bool) override
    {
        return true;
    }

    bool number_integer(number_integer_t) override
    {
        return true;
    }

    bool number_unsigned(number_unsigned_t) override
    {
        return true;
    }

    bool number_float(number_float_t, const string_t&) override
    {
        return true;
    }

    bool string(string_t&) override
    {
        return true;
    }

    bool start_object(std::size_t) override
    {
        return true;
    }

    bool key(string_t&) override
    {
        return true;
    }

    bool end_object() override
    {
        return true;
    }

    bool start_array(std::size_t) override
    {
        return true;
    }

    bool end_array() override
    {
        return true;
    }

    bool parse_error(const std::string&, const detail::exception&) override
    {
        return false;
    }
};
}

}
