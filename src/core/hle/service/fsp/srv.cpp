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

class IStorage final : public ServiceFramework<IStorage> {
public:
    IStorage(std::string file) : ServiceFramework("IStorage") {
        static const FunctionInfo functions[] = {
            {0, &IStorage::Read, "Read"},       {1, &IStorage::Write, "Write"},
            {2, &IStorage::Flush, "Flush"},     {3, &IStorage::SetSize, "SetSize"},
            {4, &IStorage::GetSize, "GetSize"},
        };
        RegisterHandlers(functions);
        fp = fopen(file.c_str(), "rb");
        isFileOpen = fp != NULL;
    }

private:
    FILE* fp;
    bool isFileOpen = false;
    void Read(Kernel::HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        u64 offset = rp.Pop<u64>();
        u64 length = rp.Pop<u64>();
        LOG_WARNING(Service, "(STUBBED) called off: 0x%lx, len: 0x%lx", offset, length);
        if (!isFileOpen) {
            IPC::RequestBuilder rb{ctx, 2};
            rb.Push(0x1F4202); // Temp for now
        }
        _fseeki64(fp, offset, SEEK_SET);

        auto output_buffer = ctx.BufferDescriptorB()[0];
        std::vector<u8> output(output_buffer.Size());
        fread(reinterpret_cast<char*>(&output[0]), length, 1, fp);
        Memory::WriteBlock(output_buffer.Address(), output.data(), output_buffer.Size());

        IPC::RequestBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
    }

    void Write(Kernel::HLERequestContext& ctx) {
        IPC::RequestBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
        LOG_WARNING(Service, "(STUBBED) called");
    }

    void Flush(Kernel::HLERequestContext& ctx) {
        IPC::RequestBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
        LOG_WARNING(Service, "(STUBBED) called");
    }

    void SetSize(Kernel::HLERequestContext& ctx) {
        IPC::RequestBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
        LOG_WARNING(Service, "(STUBBED) called");
    }

    void GetSize(Kernel::HLERequestContext& ctx) {
        IPC::RequestBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
        LOG_WARNING(Service, "(STUBBED) called");
    }
};

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
    rb.PushIpcInterface<IStorage>("RomFS.istorage"); // Get actual rom dir and check there
    LOG_WARNING(Service, "(STUBBED) called");
}

void FSP_SRV::OpenRomStorage(Kernel::HLERequestContext& ctx) {
    IPC::RequestBuilder rb{ctx, 2, 0, 0, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushIpcInterface<IStorage>("RomFS.istorage"); // Get actual rom dir and check there
    LOG_WARNING(Service, "(STUBBED) called");
}

} // namespace Filesystem
} // namespace Service
