// Copyright 2019 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/logging/log.h"
#include "core/hle/hooks/function.h"
#include "core/hle/hooks/manager.h"
#include "core/hle/hooks/stdlib/stdlib.h"

namespace Hooks {

void Manager::RegisterImport(Import&& symbol) {
    imports.emplace_back(symbol);
}

void Manager::RegisterFunction(const Symbol& symbol, Function&& func) {
    function_map[symbol.name] = std::move(func);
}

void Manager::ReloadHooks(Core::System& system) {
    stdlib::RegisterFunctions(*this);

    for (const auto& import : imports) {
        auto search = function_map.find(import.symbol.name);
        if (search == function_map.end()) {
            // No register function matches this symbol
            continue;
        }

        Hooks::AddHLEFunction(system, import.addr, search->second);

        LOG_CRITICAL(Core, "Hooking {} at 0x{:08X}", import.symbol.name, import.addr);
    }
}

} // namespace Hooks
