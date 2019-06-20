// Copyright 2019 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <vector>
#include "common/common_types.h"

namespace Loader::NSO {

void ParseDynamicSection(std::vector<u8>& program_image, u32 dynamic_section_offset,
                         VAddr load_base, Hooks::Manager& hooks_manager);

} // namespace Loader::NSO
