#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

using u8 = std::uint8_t;
using s32 = std::int32_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

namespace HLE {

enum class ArgumentType {
    Integer,
    Float,
};

struct Function {
    void* pointer;
    std::optional<ArgumentType> return_type;
    std::vector<ArgumentType> argument_types;
};

} // namespace HLE

namespace detail {

template <typename T>
std::optional<HLE::ArgumentType> GetArgumentType() {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<void, T>);
    if constexpr (std::is_integral_v<T>) {
        return HLE::ArgumentType::Integer;
    } else if constexpr (std::is_floating_point_v<T>) {
        return HLE::ArgumentType::Float;
    } else {
        return {};
    }
}

template <typename F>
struct MakeHLEFunctionImpl;

template <typename R, typename... A>
struct MakeHLEFunctionImpl<R(A...)> {
    static HLE::Function Make(R (*f)(A...)) {
        return {
            reinterpret_cast<void*>(f),
            GetArgumentType<R>(),
            {*GetArgumentType<A>()...},
        };
    }
};

} // namespace detail

template <typename F>
HLE::Function MakeHLEFunction(F* f) {
    return detail::MakeHLEFunctionImpl<F>::Make(f);
}

// TEST:

u64 foo(u64, u64, u64) {
    return 0;
}

const char* ToStr(HLE::ArgumentType a) {
    switch (a) {
    case HLE::ArgumentType::Integer:
        return "Integer";
    case HLE::ArgumentType::Float:
        return "Float";
    }
    return "<unknown>";
}

int main() {
    const auto f = MakeHLEFunction(&foo);
    printf("number of arguments: %zu\n", f.argument_types.size());
    printf("return value type: %s\n", ToStr(*f.return_type));
    printf("argument type: %s\n", ToStr(f.argument_types[0]));
}
