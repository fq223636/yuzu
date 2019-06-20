// Copyright 2019 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <optional>
#include <type_traits>
#include <vector>

#include "common/common_types.h"

namespace Core {
class System;
}

namespace Hooks {

enum class ArgumentType {
    Integer,
    Float,
};

struct Function {
    void* pointer;
    std::optional<ArgumentType> return_type;
    std::vector<ArgumentType> argument_types;
};

namespace detail {

template <typename T>
std::optional<ArgumentType> GetArgumentType() {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<void, T>);
    if constexpr (std::is_integral_v<T>) {
        return ArgumentType::Integer;
    } else if constexpr (std::is_floating_point_v<T>) {
        return ArgumentType::Float;
    } else {
        return {};
    }
}

template <typename F>
struct MakeFunctionImpl;

template <typename R, typename... A>
struct MakeFunctionImpl<R(A...)> {
    static Function Make(R (*f)(A...)) {
        return {
            reinterpret_cast<void*>(f),
            GetArgumentType<R>(),
            {*GetArgumentType<A>()...},
        };
    }
};

} // namespace detail

template <typename F>
Function MakeFunction(F* f) {
    return detail::MakeFunctionImpl<F>::Make(f);
}

void AddHLEFunction(Core::System& system, VAddr hook_addr, Function function);

} // namespace Hooks
