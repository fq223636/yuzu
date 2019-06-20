// Copyright 2019 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common/common_types.h"
#include "core/hle/hooks/function.h"

namespace Core {
class System;
}

namespace Hooks {

// This is SDK add-on version, taken from NCA header, offset 0x21C, size 0x4
// https://switchbrew.org/wiki/NCA_Format#Header
using SdkVersion = std::uint32_t;

struct Symbol {
    std::string name;
    std::optional<SdkVersion> sdk_version_min;
    std::optional<SdkVersion> sdk_version_max;
};

struct Import {
    VAddr addr;
    Symbol symbol;
};

class Manager final {
public:
    void RegisterImport(Import&& symbol);

    void RegisterFunction(const Symbol& symbol, Function&& func);

    void ReloadHooks(Core::System& system);

private:
    std::vector<Import> imports;
    std::map<std::string, Function> function_map;
};

} // namespace Hooks
