// Copyright 2019 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/arm/arm_interface.h"
#include "core/core.h"
#include "core/hle/hooks/function.h"

namespace Hooks {

void AddHLEFunction(Core::System& system, VAddr hook_addr, Function function) {
    system.ArmInterface(0).AddHLEFunction(hook_addr, function);
    system.ArmInterface(1).AddHLEFunction(hook_addr, function);
    system.ArmInterface(2).AddHLEFunction(hook_addr, function);
    system.ArmInterface(3).AddHLEFunction(hook_addr, function);
}

} // namespace Hooks
