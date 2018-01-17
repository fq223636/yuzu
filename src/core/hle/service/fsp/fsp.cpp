// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/fsp/fsp.h"
#include "core/hle/service/fsp/srv.h"

namespace Service {
namespace Filesystem {

void InstallInterfaces(SM::ServiceManager& service_manager) {
    std::make_shared<FSP_SRV>()->InstallAsService(service_manager);
}

} // namespace Filesystem
} // namespace Service
