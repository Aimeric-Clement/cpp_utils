#ifndef UTL_TRACEABLE_OPTIONAL_HPP
#define UTL_TRACEABLE_OPTIONAL_HPP

#include <stdexcept>
#include <string>
#include <optional>

namespace utl
{
    /**
     * @brief Wrapper for std::optional that can hold an error message
     * @details Wrapper for std::optional, that can also store an alternative error message
     * in case the value is absent.
     */
    template <class T>
    class traceable_opt
    {
    private:
        std::string m_err_msg = "";
        std::optional<T> m_opt = std::nullopt;
     public:
        traceable_opt() = default;
        traceable_opt(const T &&payload) : m_opt(std::move(payload)) {}
        traceable_opt(const T &payload) : m_opt(payload) {}
        template <typename = std::enable_if_t<!std::is_convertible_v<T, std::string>>> // such that it won't build an empty optional with an error when using it with strings (or related)
        traceable_opt(const std::string &&err_msg): m_err_msg(std::move(err_msg)) {}
        traceable_opt(const traceable_opt<T> &other): m_err_msg(other.m_err_msg), m_opt(other.m_opt) {}
         void set_why(const std::string &&err_msg) { m_err_msg = err_msg; }
         [[nodiscard]] std::string why() const
        {
            if (m_opt.has_value())
                throw std::logic_error("The object exists and there is therefore no error message to be retrieved");
            return m_err_msg;
        }
         [[nodiscard]] T value() const
        {
            if (m_opt.has_value())
                return m_opt.value();
            throw std::logic_error("Does not hold a value, the error was: " + m_err_msg);
        }
         [[nodiscard]] T value_or(T &&def) const { return m_opt.value_or(std::move(def)); }
        void emplace(T &&payload) { m_opt.emplace(std::move(payload)); }
        void reset() { m_opt.reset(); }
        [[nodiscard]] bool has_value() const { return m_opt.has_value(); }
        [[nodiscard]] operator std::optional<T>() const { return std::optional<T>{m_opt}; }
    };
}
#endif // UTL_TRACEABLE_OPTIONAL_HPP