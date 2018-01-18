// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/assert.h"
#include "common/logging/log.h"
#include "core/hle/service/nvdrv/devices/nvhost_ctrl.h"

namespace Service {
namespace Nvidia {
namespace Devices {

u32 nvhost_ctrl::ioctl(u32 command, const std::vector<u8>& input, std::vector<u8>& output) {
    LOG_WARNING(Debug_GPU, "Got Ioctl 0x%llx", command);
    UNIMPLEMENTED();
    return 0;
}

} // namespace Devices
} // namespace Nvidia
} // namespace Service
