// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/service.h"

namespace Service {
namespace Filesystem {

class IStorage final : public ServiceFramework<IStorage> {
public:
    IStorage();

private:
    void Read(Kernel::HLERequestContext& ctx);
    void Write(Kernel::HLERequestContext& ctx);
    void Flush(Kernel::HLERequestContext& ctx);
    void SetSize(Kernel::HLERequestContext& ctx);
    void GetSize(Kernel::HLERequestContext& ctx);
};

class FSP_SRV final : public ServiceFramework<FSP_SRV> {
public:
    FSP_SRV();
    ~FSP_SRV() = default;

private:
    void Initalize(Kernel::HLERequestContext& ctx);
    void GetGlobalAccessLogMode(Kernel::HLERequestContext& ctx);
    void OpenDataStorageByCurrentProcess(Kernel::HLERequestContext& ctx);
    void OpenRomStorage(Kernel::HLERequestContext& ctx);
};

} // namespace Filesystem
} // namespace Service
