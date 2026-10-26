#include <iostream>
#include <string>
#include <variant>

#include "containers/traceable_optional.hpp"
#include "containers/bidirectional_map.hpp"
#include "containers/interval_map.hpp"
#include "containers/bloom_filter.hpp"
#include "async_utils.hpp"
#include "variant_utils.hpp"
#include "parsing_utils.hpp"
#include "template_utils.hpp"

using foo_t = struct { int a; bool b; double d; };
using bar_t = utl::tuple_from_aggregate_t<foo_t>;

int main()
{
    std::cout << "aggregate member count: " << utl::aggregate_member_count_t<foo_t>::value << "\n";

    bar_t bar = {10, false, 1.0};
    std::cout << "bar tuple values = ("
              << std::get<0>(bar) << ", "
              << std::boolalpha << std::get<1>(bar) << ", "
              << std::get<2>(bar) << ")\n";

    utl::traceable_opt<int> present_value = 42;
    utl::traceable_opt<int> absent_value("no value provided");
    std::cout << "present_value = " << present_value.value_or(-1) << "\n";
    if (!absent_value.has_value())
        std::cout << "absent_value reason: " << absent_value.why() << "\n";

    utl::bidirectional_map<std::string, int> mapping{{"one", 1}, {"two", 2}, {"three", 3}};
    std::cout << "mapping size = " << mapping.size() << "\n";
    if (auto it = mapping.direct().find("two"); it != mapping.direct().end())
        std::cout << "two -> " << it->second << "\n";
    if (auto it = mapping.invert().find(3); it != mapping.invert().end())
        std::cout << "3 -> " << it->second << "\n";

    utl::interval_map<int, char> intervals('.');
    intervals.assign(1, 5, 'A');
    intervals.assign(3, 7, 'B');
    std::cout << "interval map: ";
    for (int i = 0; i <= 8; ++i)
        std::cout << intervals[i];
    std::cout << "\n";

    utl::bloom_filter<std::string> bloom;
    bloom.insert("hello");
    std::cout << "bloom contains 'hello' = " << std::boolalpha << bloom.contains("hello")
              << ", 'world' = " << bloom.contains("world") << "\n";

    std::variant<int, std::string, bool> variant_value = std::string("hello variant");
    std::cout << "variant to_string = " << utl::to_string(variant_value, "<unsupported>") << "\n";
    std::cout << "variant ostream = " << utl::variant_ostream(variant_value) << "\n";

    utl::jthread worker([](){ std::cout << "background task executed\n"; });
    worker.join();

    std::cout << "trimmed: '" << utl::trim_spaces("   spaced text   ") << "'\n";
    std::cout << "loosely_match(' true ', 'TRUE') = " << utl::loosely_match(" true ", "TRUE") << "\n";

    auto safe_int = utl::default_on_fail(-1, [](){ throw std::runtime_error("fail"); return 1; });
    std::cout << "default_on_fail result = " << safe_int << "\n";

    return 0;
}
