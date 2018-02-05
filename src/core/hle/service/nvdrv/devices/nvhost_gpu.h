// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <vector>
#include "common/common_types.h"
#include "common/swap.h"
#include "core/hle/service/nvdrv/devices/nvdevice.h"

namespace Service {
namespace Nvidia {
namespace Devices {

class nvhost_gpu final : public nvdevice {
public:
    nvhost_gpu() = default;
    ~nvhost_gpu() override = default;

    u32 ioctl(u32 command, const std::vector<u8>& input, std::vector<u8>& output) override;

private:
    u32 nvmap_fd{};
    u64_le user_data{};

    enum IoctlCommands {
        IocSetNVMAPfdCommand = 0x40044801,
        IocAllocGPFIFOEx2Command = 0xC020481A,
        IocAllocObjCtxCommand = 0xC0104809,
        IocSetErrorNotifierCommand = 0xC018480C,
        IocSetClientDataCommand = 0x40084714,
        IocGetClientDataCommand = 0x80084715,
        IocChannelSetPriorityCommand = 0x4004480D,
    };

private:
    struct channel_priority {
        u32_le priority;
    };

    struct client_data {
        u64_le data;
    };

    struct set_error_notifier {
        u64_le offset;
        u64_le size;
        u32_le mem; // nvmap object handle
        u32_le padding;
    };

    struct set_nvmap_fd {
        u32_le nvmap_fd;
    };

    struct fence {
        u32 id;
        u32 value;
    };

    struct alloc_gpfifo_ex2 {
        u32_le __num_entries;     // in
        u32_le __flags;           // in
        u32_le __unk0;            // in (1 works)
        struct fence __fence_out; // out
        u32_le __unk1;            // in
        u32_le __unk2;            // in
        u32_le __unk3;            // in
    };

    struct alloc_obj_ctx {
        u32_le class_num; // 0x902D=2d, 0xB197=3d, 0xB1C0=compute, 0xA140=kepler, 0xB0B5=DMA,
                          // 0xB06F=channel_gpfifo
        u32_le flags;
        u64_le obj_id; // (ignored) used for FREE_OBJ_CTX ioctl, which is not supported
    };

    struct gpfifo_entry {
        u32 entry0; // gpu_va_lo
        u32 entry1; // gpu_va_hi | (unk_0x02 << 0x08) | (size << 0x0A) | (unk_0x01 << 0x1F)
    };

    struct submit_gpfifo {
        u64 gpfifo;      // (ignored) pointer to gpfifo fence structs
        u32 num_entries; // number of fence objects being submitted
        u32 flags;
        struct fence fence_out;        // returned new fence object for others to wait on
        struct gpfifo_entry entries[]; // depends on num_entries
    };

    u32 SetNVMAPfd(const std::vector<u8>& input, std::vector<u8>& output);
    u32 AllocGPFIFOEx2(const std::vector<u8>& input, std::vector<u8>& output);
    u32 AllocateObjectContext(const std::vector<u8>& input, std::vector<u8>& output);
    u32 SetErrorNotifier(const std::vector<u8>& input, std::vector<u8>& output);
    u32 SetClientData(const std::vector<u8>& input, std::vector<u8>& output);
    u32 GetClientData(const std::vector<u8>& input, std::vector<u8>& output);
    u32 SetChannelPriority(const std::vector<u8>& input, std::vector<u8>& output);
    u32 SubmitGPFIFO(const std::vector<u8>& input, std::vector<u8>& output);
};

} // namespace Devices
} // namespace Nvidia
} // namespace Service
