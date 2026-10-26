#ifndef CUSTOM_EXCEPTION_UTILS
#define CUSTOM_EXCEPTION_UTILS

#include <stdexcept>

namespace utl
{
    class not_implemented_exception : public std::logic_error
    {
    public:
        not_implemented_exception() : std::logic_error("Has yet to be implemented") { }
    };

    class aborted_exception : public std::runtime_error
    {
    public:
        aborted_exception() : std::runtime_error("aborted") { }
        aborted_exception(std::string&& what) : std::runtime_error(what) { }
        aborted_exception(const std::string& what) : std::runtime_error(what) { }
        aborted_exception(const char* what) : std::runtime_error(what) { }
    };
}

#endif
