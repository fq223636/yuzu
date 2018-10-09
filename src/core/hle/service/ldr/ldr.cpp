// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
#pragma optimize("", off)
#include <memory>

#include "core/core.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/process.h"
#include "core/hle/service/ldr/ldr.h"
#include "core/hle/service/service.h"

namespace Service::LDR {

class DebugMonitor final : public ServiceFramework<DebugMonitor> {
public:
    explicit DebugMonitor() : ServiceFramework{"ldr:dmnt"} {
        // clang-format off
        static const FunctionInfo functions[] = {
            {0, nullptr, "AddProcessToDebugLaunchQueue"},
            {1, nullptr, "ClearDebugLaunchQueue"},
            {2, nullptr, "GetNsoInfos"},
        };
        // clang-format on

        RegisterHandlers(functions);
    }
};

class ProcessManager final : public ServiceFramework<ProcessManager> {
public:
    explicit ProcessManager() : ServiceFramework{"ldr:pm"} {
        // clang-format off
        static const FunctionInfo functions[] = {
            {0, nullptr, "CreateProcess"},
            {1, nullptr, "GetProgramInfo"},
            {2, nullptr, "RegisterTitle"},
            {3, nullptr, "UnregisterTitle"},
        };
        // clang-format on

        RegisterHandlers(functions);
    }
};

class Shell final : public ServiceFramework<Shell> {
public:
    explicit Shell() : ServiceFramework{"ldr:shel"} {
        // clang-format off
        static const FunctionInfo functions[] = {
            {0, nullptr, "AddProcessToLaunchQueue"},
            {1, nullptr, "ClearLaunchQueue"},
        };
        // clang-format on

        RegisterHandlers(functions);
    }
};

class RelocatableObject final : public ServiceFramework<RelocatableObject> {
public:
    explicit RelocatableObject() : ServiceFramework{"ldr:ro"} {
        // clang-format off
        static const FunctionInfo functions[] = {
            {0, &RelocatableObject::LoadNro, "LoadNro"},
            {1, nullptr, "UnloadNro"},
            {2, &RelocatableObject::LoadNrr, "LoadNrr"},
            {3, nullptr, "UnloadNrr"},
            {4, &RelocatableObject::Initialize, "Initialize"},
        };
        // clang-format on

        RegisterHandlers(functions);
    }

    void LoadNrr(Kernel::HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        rp.Skip(2, false);
        auto address = rp.Pop<VAddr>();
        auto size = rp.Pop<u64>();

        auto process = Core::CurrentProcess();

        auto map_address = process->VMManager().FindFreeRegion(size);

        ASSERT(map_address.Succeeded());

        auto result = process->MirrorMemory(*map_address, address, size);
        ASSERT(result == RESULT_SUCCESS);

        IPC::ResponseBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
        LOG_WARNING(Service, "called");
    }

    struct NroHeader {
        u32 entrypoint_insn;
        u32 mod_offset;
        u64 padding;
        u32 magic;
        u32 _0x14;
        u32 nro_size;
        u32 _0x1C;
        u32 text_offset;
        u32 text_size;
        u32 ro_offset;
        u32 ro_size;
        u32 rw_offset;
        u32 rw_size;
        u32 bss_size;
        u32 _0x3C;
        u8 build_id[0x20];
        u8 _0x60[0x20];
    };

    void LoadNro(Kernel::HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        rp.Skip(2, false);
        auto nro_address = rp.Pop<VAddr>();
        auto nro_size = rp.Pop<u64>();
        auto bss_address = rp.Pop<VAddr>();
        auto bss_size = rp.Pop<u64>();

        NroHeader nro_header;
        Memory::ReadBlock(nro_address, &nro_header, sizeof(nro_header));

        auto process = Core::CurrentProcess();

        auto map_address = process->VMManager().FindFreeRegion(nro_size + bss_size);

        ASSERT(map_address.Succeeded());

        auto result = process->MirrorMemory(*map_address, nro_address, nro_size);
        ASSERT(result == RESULT_SUCCESS);

        if (bss_size > 0) {
            result = process->MirrorMemory(*map_address + nro_size, bss_address, bss_size);
            ASSERT(result == RESULT_SUCCESS);
        }

        process->VMManager().ReprotectRange(*map_address, nro_header.text_size,
                                            Kernel::VMAPermission::ReadExecute);
        process->VMManager().ReprotectRange(*map_address + nro_header.ro_offset, nro_header.ro_size,
                                            Kernel::VMAPermission::Read);
        process->VMManager().ReprotectRange(*map_address + nro_header.rw_offset,
                                            nro_header.rw_size + bss_size,
                                            Kernel::VMAPermission::ReadWrite);

        Core::System::GetInstance().ArmInterface(0).ClearInstructionCache();
        Core::System::GetInstance().ArmInterface(1).ClearInstructionCache();
        Core::System::GetInstance().ArmInterface(2).ClearInstructionCache();
        Core::System::GetInstance().ArmInterface(3).ClearInstructionCache();

        IPC::ResponseBuilder rb{ctx, 4};
        rb.Push(RESULT_SUCCESS);
        rb.Push(nro_address);

        LOG_WARNING(Service, "called");
    }

    void Initialize(Kernel::HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};

        IPC::ResponseBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
        LOG_WARNING(Service, "called");
    }
};

void InstallInterfaces(SM::ServiceManager& sm) {
    std::make_shared<DebugMonitor>()->InstallAsService(sm);
    std::make_shared<ProcessManager>()->InstallAsService(sm);
    std::make_shared<Shell>()->InstallAsService(sm);
    std::make_shared<RelocatableObject>()->InstallAsService(sm);
}

} // namespace Service::LDR
