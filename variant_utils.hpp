#ifndef VARIANT_UTILS_AC_HPP
#define VARIANT_UTILS_AC_HPP

#include "template_utils.hpp"
#include "exceptions.hpp"

#include <variant>
#include <locale>
#include <string>

/**
 * @brief contains a number of runtime tools, meta-functions helpers and useful container types
 * 
 * namely,
 *  to_string:
 *      A helper to convert the content of a std::variant<T> to std::string for any T,
 *      quietly ignoring unsupported types.
 * 
 *  convert_variant:
 *      Extracts the value from a variant to insert it back into a different variant, with a
 *      different type. In case the destination variant does not map all the types from the
 *      origin and that the contained value type is not found in the destination;
 *      throws an error.
 * 
 *  variant_ostream:
 *      An ostream adaptor that will compile regardless of the underlying types.
 *      It streams out the content of a variant, or fails at runtime.
 */
namespace utl
{
    namespace impl
    {
        template <typename T, typename = void>
        struct is_to_string_supported
        : std::false_type {};

        template <typename T>
        struct is_to_string_supported <T, std::void_t<decltype(std::to_string(std::declval<T>()))>>
        : std::true_type {};

        template <typename T, typename = void>
        struct is_ostream_supported
        : std::false_type {};

        template <typename T>
        struct is_ostream_supported <T, std::void_t<decltype(std::declval<std::ostream>() << std::declval<T>())>>
        : std::true_type {};
    }

    /**
     * @brief resiliently convert the content of a variant to a string
     * @return if the type can be converted, then the output is the same as calling std::to_string on that type,
     * otherwise it defaults on the "not_supported" argument.
     * 
     * @note this helper is not meant for any other purpose than having an easier time debugging,
     * please consider using a tailor-made solution for production code instead, adding an
     * independent specialization for each types of the variant.
     */
    template <typename... Ts>
    [[nodiscard]] inline std::string to_string(const std::variant<Ts...>& variant, const std::string& not_supported = "")
    {
        return std::visit([&not_supported](auto&& v)-> std::string
        {
            std::string ret;
            if constexpr (std::is_same<utl::remove_cvref_t<decltype(v)>, std::string>::value)
                return v;
            
            else if constexpr (std::is_same<utl::remove_cvref_t<decltype(v)>, bool>::value)
                return v? "true"
                        : "false";

            else if constexpr (impl::is_to_string_supported<decltype(v)>::value)
                return std::to_string(std::forward<decltype(v)>(v));

            else
                return not_supported;
        }, std::forward<decltype(variant)>(variant));
    }

    /**
     * @brief cast the variant to any other variant type
     * @details cast any variant to any other variant type, so long as the current held value
     * has its type supported in the destination, otherwise throw a std::bad_variant_access exception
     * @tparam R the destination variant type
     */
    template <typename R, typename V>
    [[nodiscard]] inline R convert_variant(V&& v)
    {
        static_assert(has_unique_types_v<utl::remove_cvref_t<V>>, "The source variant type cannot hold the same type more than once");
        static_assert(has_unique_types_v<R>, "The target variant type cannot hold the same type more than once");
        static_assert(is_template_type_specialisation_v<std::variant, utl::remove_cvref_t<V>>, "The source type must be a std::variant");
        static_assert(is_template_type_specialisation_v<std::variant, R>, "The target type must be a std::variant");
        return std::visit(
            [](auto&& val) -> R
            {
                using val_t = utl::remove_cvref_t<decltype(val)>;
                if constexpr (contains_type_v<val_t, R>)
                    return R(std::in_place_type<val_t>, std::forward<decltype(val)>(val));
                else
                    // alternative type from the origin does not exist in the target variant
                    throw std::bad_variant_access();
            }, std::forward<V>(v));
    }

    /**
     * @brief cast the variant to any other variant type, with a default fallback value
     * @details cast any variant to any other variant type, so long as the current held value
     * has its type supported in the destination, otherwise return the user defined fallback value
     * @tparam R the destination variant type
     */
    template <typename R, typename V, typename T>
    [[nodiscard]] inline R convert_variant_or(V&& v, T&& fallback)
    {
        static_assert(has_unique_types_v<utl::remove_cvref_t<V>>, "The source variant type cannot hold the same type more than once");
        static_assert(has_unique_types_v<R>, "The target variant type cannot hold the same type more than once");
        static_assert(contains_type_v<utl::remove_cvref_t<T>, R>, "The fallback value must be of a type supported by the target variant type");
        static_assert(is_template_type_specialisation_v<std::variant, utl::remove_cvref_t<V>>, "The source type must be a std::variant");
        static_assert(is_template_type_specialisation_v<std::variant, R>, "The target type must be a std::variant");

        return std::visit(
            [fallback = std::move(fallback)](auto&& val) -> R
            {
                using val_t = utl::remove_cvref_t<decltype(val)>;
                if constexpr (contains_type_v<val_t, R>)
                    return R(std::in_place_type<val_t>, std::forward<decltype(val)>(val));
                else
                    // alternative type from the origin does not exist in the target variant
                    return R(std::in_place_type<T>, std::forward<decltype(fallback)>(fallback));
            }, std::forward<V>(v));
    }

    /**
     * @brief Thin wrapper to allow streaming a std::variant specialization to ostream
     * @exception utl::not_implemented_exception whether the actual streamed type does not implement the operator to do so
     */
    template <typename T>
    class variant_ostream
    {
        static_assert(is_template_type_specialisation_v<std::variant, utl::remove_cvref_t<T>>, "The ostream template parameter must be a std::variant specialisation");
    private:
        const T& m_v;
        friend std::ostream& operator<<(std::ostream& lhs, const variant_ostream& rhs)
        {
            std::visit([&lhs](auto&& rhs_content)
            {
                if constexpr (impl::is_ostream_supported<decltype(rhs_content)>::value)
                    lhs << std::forward<decltype(rhs_content)>(rhs_content);
                else
                    throw utl::not_implemented_exception(); // No ADL-accesible ostream& operator<< is implemented for the given type
            }, rhs.m_v);
            return lhs;
        }

    public:
        variant_ostream(T&& v) : m_v(v) {}
        variant_ostream(T& v) : m_v(v) {}
    };
}

#endif