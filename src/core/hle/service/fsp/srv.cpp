// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/logging/log.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/client_port.h"
#include "core/hle/kernel/client_session.h"
#include "core/hle/service/fsp/srv.h"

namespace Service {
namespace Filesystem {

IStorage::IStorage() : ServiceFramework("IStorage") {
    static const FunctionInfo functions[] = {
        {0, &IStorage::Read, "Read"},       {1, &IStorage::Write, "Write"},
        {2, &IStorage::Flush, "Flush"},     {3, &IStorage::SetSize, "SetSize"},
        {4, &IStorage::GetSize, "GetSize"},
    };
    RegisterHandlers(functions);
}

void IStorage::Read(Kernel::HLERequestContext& ctx) {

    IPC::RequestParser rp{ctx};
    u32 offset = rp.Pop<u32>();
    u32 length = rp.Pop<u32>();

    IPC::RequestBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service, "(STUBBED) called off: 0x%llx, len: 0x%llx", offset, length);
}

void IStorage::Write(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service, "(STUBBED) called");
}

void IStorage::Flush(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service, "(STUBBED) called");
}

void IStorage::SetSize(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service, "(STUBBED) called");
}

void IStorage::GetSize(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service, "(STUBBED) called");
}

FSP_SRV::FSP_SRV() : ServiceFramework("fsp-srv") {
    static const FunctionInfo functions[] = {
        {1, &FSP_SRV::Initalize, "Initalize"},
        {200, &FSP_SRV::OpenDataStorageByCurrentProcess, "OpenDataStorageByCurrentProcess"},
        {203, &FSP_SRV::OpenRomStorage, "OpenRomStorage"},
        {1005, &FSP_SRV::GetGlobalAccessLogMode, "GetGlobalAccessLogMode"},
    };
    RegisterHandlers(functions);
}

void FSP_SRV::Initalize(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service, "(STUBBED) called");
}

void FSP_SRV::GetGlobalAccessLogMode(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 4};
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(5);
    LOG_WARNING(Service, "(STUBBED) called");
}

void FSP_SRV::OpenDataStorageByCurrentProcess(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2, 0, 0, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushIpcInterface<IStorage>();
    LOG_WARNING(Service, "(STUBBED) called");
}

void FSP_SRV::OpenRomStorage(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2, 0, 0, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushIpcInterface<IStorage>();
    LOG_WARNING(Service, "(STUBBED) called");
}

} // namespace Filesystem
} // namespace Service
