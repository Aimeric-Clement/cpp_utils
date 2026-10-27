#ifndef TEMPLATE_UTILS_AC_HPP
#define TEMPLATE_UTILS_AC_HPP

#include <utility>
#include <exception>
#include <functional>
#include <type_traits>
#include <tuple>

#include "exceptions.hpp"

/**
 * @brief contains a number of runtime tools, meta-functions helpers and useful container types
 * 
 * namely,
 *  scoped_resource_wrapper:
 *      A simple object class for RAII, to run some function when going out of scope.
 *      It may be used to handle C-style ressources that are not owned and must be closed
 *      at the end of the scope and when some exception is raised.

 *  generic exception:
 *      not_implemented_exception, for features that are in the work.
 * 
 *  traceable_opt:
 *      A std::optional wrapper that contains an alternative text message. The intended
 *      use for it is to deal with exceptions as values and trace what happened when something
 *      went wrong.
 * 
 *  overloaded_fn:
 *      Declare a lambda function with several signatures. It is convenient to perform
 *      less verbose type resolutions on variant types.
 * 
 *  contains_type:
 *      Meta-function to check if some type exists in a variant type, a tuple, or various
 *      other templated containers types. Can also be used directly
 *      with utl::contains_type_v.
 * 
 *  bidirectional_map:
 *      A container enforcing a bijective maping of the key and value
 *      pairs of two maps. It allows an efficient reverse lookups (key from value) using the
 *      member function invert(), and forward lookup using direct().
 * 
 *  flatten:
 *      Make a larger templated container out of many
 * 
 *  extends:
 *      Extends a templated container with the given extra types
 * 
 *  remove_cvref:
 *      Same as in c++20
 * 
 *  is_template_type_specialization:
 *      A templated type trait interface, to check that the given type is from some generic template.
 * 
 *  interval_map:
 *      An associative container linking bounded ranges (the keys) to values. Lookups are performed in logarithmic time.
 *      Eg: it may be used to efficiently store segments of a metro line, relative to a one-dimensional location.
 */
namespace utl
{
    /**
     * @brief a function with inline function signature overloads for type resolution
     * @tparam T... Function types redefining the behavior for a given argument type
     * 
     * @example
     * std::visit(overloaded_fn{
     *  [](std::monostate&){...},
     *  [](T&){...},
     *  [](U&){...},
     *  [](C&){...}
     * }, foo_variant);
     */
    template<class... Ts> struct overloaded_fn : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded_fn(Ts...) -> overloaded_fn<Ts...>;

    /**
     * @brief if C contains the type name of T, returns a true_type. otherwise a false_type
     * @tparam T type to be contained or not in C
     * @tparam C templatized meta container
     */
    template <typename T, template <typename...> class C, typename ... Ts>
    constexpr auto contains_type (C<Ts...> const &) -> std::disjunction<std::is_same<T, Ts>...> { return {}; }; // FIXME: it would better be a more standardized integral constant type

    /**
     * @brief check if type name T is contained in the templatized container type C
     * @note is a function helper for the struct utl::contain_type
     * 
     * @example
     * using foovar = std::variant<int, bool>;
     * static_assert(contains_type<int, foovar>::value); 
     * 
     * @tparam T type to be contained or not in C
     * @tparam C templatized meta container
     */
    template <typename T, typename V>
    static constexpr bool contains_type_v = decltype(contains_type<T>(std::declval<V>()))::value;

    /**
     * @brief strip the type of all qualifiers
     * @note as in c++20
     */
    template<typename T>
    struct remove_cvref
    {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };

    /**
     * @brief strip the type of all qualifiers
     * @note as in c++20
     */
    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    /**
     * @brief check that each types appears only once amongst the template arguments
     * @code
     * is_unique_type_list<int, int>::value // is false
     * is_unique_type_list<int, float>::value // is true
     * @endcode
     */
    template <typename...>
    struct is_unique_type_list : std::false_type {};
    /**
     * @brief check that each types appears only once amongst the template arguments
     * @code
     * is_unique_type_list<int, int>::value // is false
     * is_unique_type_list<int, float>::value // is true
     * @endcode
     */
    template <typename T>
    struct is_unique_type_list<T> : std::true_type {};
    /**
     * @brief check that each types appears only once amongst the template arguments
     * @code
     * is_unique_type_list<int, int>::value // is false
     * is_unique_type_list<int, float>::value // is true
     * @endcode
     */
    template <typename T, typename... Rest>
    struct is_unique_type_list<T, Rest...>
    : std::conjunction <std::negation <std::disjunction <std::is_same <T, Rest>...>>
                        , is_unique_type_list<Rest...>> {};

    template <typename T>
    static constexpr bool is_unique_type_list_v = is_unique_type_list<T>::value;

    template <typename T>
    struct has_unique_types : std::false_type {};

    /**
     * @brief check that each types appears only once amongst the template arguments
     * @code
     * has_unique_types<std::tuple<int, int>>::value // is false
     * has_unique_types<std::tuple<int, float>>::value // is true
     * @endcode
     */
    template <template <typename...> class TT, typename... Ts>
    struct has_unique_types<TT<Ts...>> : is_unique_type_list<Ts...> {};
    
    /**
     * @brief check that each types appears only once amongst the template arguments
     * @code
     * has_unique_types_v<std::tuple<int, int>> // is false
     * has_unique_types_v<std::tuple<int, float>> // is true
     * @endcode
     */
    template <typename T>
    static constexpr bool has_unique_types_v = has_unique_types<T>::value;

    /**
     * @brief create a templated container type out of several without nesting
     * @code
     * using a_t = std::tuple<int, double>;
     * using b_t = std::tuple<std::string, char>;
     * using a_and_b_t = flatten<a_t, b_t>::type; // is of type std::tuple<int, double, std::string, char>
     * @endcode
     */
    template <typename...>
    struct flatten;
    template <template <typename...> class C, typename... Ts>
    struct flatten<C<Ts...>>
    { using type = C<Ts...>; };
    template <template <typename...> class C, typename... Ts, typename... Us, typename... Rest>
    struct flatten<C<Ts...>, C<Us...>, Rest...>
    { using type = typename flatten<C<Ts..., Us...>, Rest...>::type; };

    /**
     * @brief create a templated container type out of several without nesting
     * @code
     * using a_t = std::tuple<int, double>;
     * using b_t = std::tuple<std::string, char>;
     * using a_and_b_t = flatten_t<a_t, b_t>; // is of type std::tuple<int, double, std::string, char>
     * @endcode
     */
    template <typename... Ts>
    using flatten_t = typename flatten<Ts...>::type;


    /**
     * @brief extends a templated container with the given extra types
     * @code
     * using a_t = std::tuple<int, double>;
     * using b_t = extends<a_t, bool, char>::type; // is of type std::tuple<int, double, bool, char>
     * @endcode
     * @tparam the first template parameter is the base container type, the rest are the types to append to that container
     */
    template <typename...>
    struct extends;
    template <template <typename...> class C, typename... Ts>
    struct extends<C<Ts...>>
    { using type = C<Ts...>; };
    template <template <typename...> class C, typename... Ts, typename... Ks>
    struct extends<C<Ts...>, Ks...>
    { using type = C<Ts..., Ks...>; };

    /**
     * @brief extends a templated container with the given extra types
     * @code
     * using a_t = std::tuple<int, double>;
     * using b_t = extends_t<a_t, bool, char>; // is of type std::tuple<int, double, bool, char>
     * @endcode
     * @tparam the first template parameter is the base container type, the rest are the types to append to that container
     */
    template <typename... Ts>
    using extends_t = typename extends<Ts...>::type;

    /**
     * @brief checks that a type is a specialisation of some template
     * 
     * @tparam TT the template
     * @tparam U the tested type
     * @code
     * is_specialisation_of_type<std::vector, std::vector<bool>> => true 
     * is_specialisation_of_type<std::vector, std::pair<bool, bool>> => false
     * @endcode
     */
    template <template <typename ...> class TT, typename U>
    struct is_template_type_specialisation : std::false_type {};
    template <template <typename ...> class TT, typename... Us>
    struct is_template_type_specialisation<TT, TT<Us...>> : std::true_type {};
    template <template <typename ...> class TT, typename U>
    inline constexpr bool is_template_type_specialisation_v = is_template_type_specialisation<TT, U>::value;

    /**
     * @brief A one-liner function wrapper to handle simple errors
     * If an exception occure in the function, its return value is replaced by a default one
     * @tparam default_result is the value that will be returned if the function fails
     * @tparam functor is an invokable object, optionaly taking arguments
     */
    template <typename F, typename... Args, typename = std::enable_if_t<std::is_invocable_v<F, Args...>>>
    auto default_on_fail(const std::invoke_result_t<F, Args...>& default_result, F functor, Args... args) -> std::invoke_result_t<F, Args...>
    {
        try
        {
            return std::invoke(std::forward<F>(functor), std::forward<Args>(args)...);
        }
        catch (const std::exception&)
        {
            return default_result;
        }
    }

    /**
     * @brief Count the members of an aggregate type
     */
    namespace impl
    {
        struct impl_dummy_convertible
        {
            template<class T>
            operator T&&() const;
        };

        template<typename /*always void*/, typename T, std::size_t... Expandor>
        struct impl_aggregate_member_count_t {
            static_assert(std::is_aggregate_v<T>, "T must be an aggregate to get the member count!");
            static constexpr std::size_t value = sizeof...(Expandor) - 1;
        };

        template<typename T, std::size_t... Expandor>
        struct impl_aggregate_member_count_t<decltype(T{(Expandor, void(), impl_dummy_convertible{})...}, void()), T, Expandor...>
        {
            static constexpr std::size_t value = impl_aggregate_member_count_t<void, T, Expandor..., 0>::value;
        };
    }

    template<typename T>
    using aggregate_member_count_t = std::integral_constant<size_t, impl::impl_aggregate_member_count_t<void, T>::value>;

    /**
     * @brief Map the types within an aggregate type onto a new tuple type
     */
    namespace impl
    {
        template <typename T>
        constexpr auto impl_returns_tuple_type_identity_from_aggregate()
        {
            const size_t member_count = aggregate_member_count_t<T>::value;
            if constexpr (member_count == 1)
            {
                auto&& [_0] = static_cast<T&&>(T{});
                return std::tuple<decltype(_0)>();
            }
            else if constexpr (member_count == 2)
            {
                auto&& [_0, _1] = static_cast<T&&>(T{});
                return std::tuple<decltype(_0), decltype(_1)>();
            }
            else if constexpr (member_count == 3)
            {
                auto&& [_0, _1, _2] = static_cast<T&&>(T{});
                return std::tuple<decltype(_0), decltype(_1), decltype(_2)>();
            }
            else
            {
                static_assert("cannot unwrap an aggregate with too many members");
                throw utl::not_implemented_exception();
            }
        }
    }

    template <typename T>
    using tuple_from_aggregate_t = decltype(impl::impl_returns_tuple_type_identity_from_aggregate<T>());

}

#endif