// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <map>
#include "common/assert.h"
#include "common/logging/log.h"
#include "core/hle/service/nvdrv/devices/nvhost_gpu.h"

namespace Service {
namespace Nvidia {
namespace Devices {

u32 nvhost_gpu::ioctl(u32 command, const std::vector<u8>& input, std::vector<u8>& output) {
    LOG_WARNING(Debug_GPU, "Got Ioctl 0x%x, inputsz: 0x%x, outputsz: 0x%x", command, input.size(),
                output.size());

    switch (command) {
    case IocSetNVMAPfdCommand:
        return SetNVMAPfd(input, output);
    case IocAllocGPFIFOEx2Command:
        return AllocGPFIFOEx2(input, output);
    case IocAllocObjCtxCommand:
        return AllocateObjectContext(input, output);
    case IocSetErrorNotifierCommand:
        return SetErrorNotifier(input, output);
    case IocSetClientDataCommand:
        return SetClientData(input, output);
    case IocGetClientDataCommand:
        return GetClientData(input, output);
    case IocChannelSetPriorityCommand:
        return SetChannelPriority(input, output);
    }

    if (((command >> 8) & 0xff) == 'H') {
        if ((command & 0xff) == 0x8) {
            return SubmitGPFIFO(input, output);
        }
    }
    printf("%x, %x\n", command & 0xff, (command >> 8) & 0xff);

    UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::SubmitGPFIFO(const std::vector<u8>& input, std::vector<u8>& output) {
    submit_gpfifo params;
    if (input.size() < 24)
        UNIMPLEMENTED();
    std::memcpy(&params, input.data(), 24);
    LOG_WARNING(Service_NVDRV, "(STUBBED) called, gpfifo=%lx, num_entries=%x, flags=%x",
                params.gpfifo, params.num_entries, params.flags);

    gpfifo_entry* entries = new gpfifo_entry[params.num_entries];
    std::memcpy(entries, &input.data()[24], params.num_entries * 8);
    for (int i = 0; i < params.num_entries; i++) {
        u32 unk1 = (entries[i].entry1 >> 8) & 0x3;
        u32 sz = (entries[i].entry1 >> 10) & 0x1fffff;
        u32 unk2 = entries[i].entry1 >> 31;
        u64 va_addr = entries[i].entry0 | ((entries[i].entry1 & 0xff) << 32);
        printf("%d - %x, %x - unk1: %x, sz: %x, unk2: %x, va_addr: %lx\n", i, entries[i].entry0,
               entries[i].entry1, unk1, sz, unk2, va_addr);
    }

    params.fence_out.id = 0;
    params.fence_out.value = 0xdeadbeef;
    std::memcpy(output.data(), &params, output.size());
    delete[] entries;
    // UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::SetChannelPriority(const std::vector<u8>& input, std::vector<u8>& output) {
    channel_priority params;
    std::memcpy(&params, input.data(), input.size());
    LOG_WARNING(Service_NVDRV, "(STUBBED) called, priority=%x", params.priority);
    std::memcpy(output.data(), &params, output.size());
    // UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::SetNVMAPfd(const std::vector<u8>& input, std::vector<u8>& output) {
    set_nvmap_fd params;
    std::memcpy(&params, input.data(), input.size());
    LOG_WARNING(Service_NVDRV, "(STUBBED) called, fd=%x", params.nvmap_fd);
    nvmap_fd = params.nvmap_fd;
    std::memcpy(output.data(), &params, output.size());
    // UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::GetClientData(const std::vector<u8>& input, std::vector<u8>& output) {
    client_data params;
    std::memcpy(&params, input.data(), input.size());
    LOG_WARNING(Service_NVDRV, "(STUBBED) called");
    params.data = user_data;
    std::memcpy(output.data(), &params, output.size());
    // UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::SetClientData(const std::vector<u8>& input, std::vector<u8>& output) {
    client_data params;
    std::memcpy(&params, input.data(), input.size());
    LOG_WARNING(Service_NVDRV, "(STUBBED) called, data=%lx", params.data);
    user_data = params.data;
    std::memcpy(output.data(), &params, output.size());
    // UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::AllocGPFIFOEx2(const std::vector<u8>& input, std::vector<u8>& output) {
    alloc_gpfifo_ex2 params;
    std::memcpy(&params, input.data(), input.size());
    LOG_WARNING(Service,
                "(STUBBED) called, num_entries=%x, flags=%x, unk0=%x, unk1=%x, unk2=%x, unk3=%x",
                params.__num_entries, params.__flags, params.__unk0, params.__unk1, params.__unk2,
                params.__unk3);
    params.__fence_out.id = 0;
    params.__fence_out.value = 0;
    std::memcpy(output.data(), &params, output.size());
    // UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::AllocateObjectContext(const std::vector<u8>& input, std::vector<u8>& output) {
    alloc_obj_ctx params;
    std::memcpy(&params, input.data(), input.size());
    LOG_WARNING(Service_NVDRV, "Allocating graphics object context, class_num=%x, flags=%x",
                params.class_num, params.flags);
    std::map<int, std::string> gfxContexts;
    gfxContexts[0x902D] = "2D";
    gfxContexts[0xB197] = "3D";
    gfxContexts[0xB1C0] = "Compute";
    gfxContexts[0xA140] = "Kepler";
    gfxContexts[0xB0B5] = "DMA";
    gfxContexts[0xB06F] = "Channel GPFIFO";

    LOG_INFO(Service_NVDRV, "Allocated graphics context \"%s\"",
             gfxContexts[params.class_num].c_str());
    params.obj_id = 0x41414141;
    std::memcpy(output.data(), &params, output.size());
    // UNIMPLEMENTED();
    return 0;
}

u32 nvhost_gpu::SetErrorNotifier(const std::vector<u8>& input, std::vector<u8>& output) {
    set_error_notifier params;
    std::memcpy(&params, input.data(), input.size());
    LOG_WARNING(Service_NVDRV, "called, offset=%lx, size=%lx, mem=%x", params.offset, params.size,
                params.mem);
    std::memcpy(output.data(), &params, output.size());
    // UNIMPLEMENTED();
    return 0;
}

} // namespace Devices
} // namespace Nvidia
} // namespace Service
