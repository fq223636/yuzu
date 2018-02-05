// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include "core/hle/service/service.h"

namespace FileSys {
class FileSystemBackend;
}

namespace Service {
namespace FileSystem {

class FSP_SRV final : public ServiceFramework<FSP_SRV> {
public:
    explicit FSP_SRV();
    ~FSP_SRV() = default;

private:
    void TryLoadRomFS();

    void Initalize(Kernel::HLERequestContext& ctx);
    void CreateSaveData(Kernel::HLERequestContext& ctx);
    void MountSaveData(Kernel::HLERequestContext& ctx);
    void GetGlobalAccessLogMode(Kernel::HLERequestContext& ctx);
    void OpenDataStorageByCurrentProcess(Kernel::HLERequestContext& ctx);
    void OpenRomStorage(Kernel::HLERequestContext& ctx);

    std::unique_ptr<FileSys::FileSystemBackend> romfs;

private:
    struct save_struct {
        u64 tid;
        u64 user_id[2];
        u64 save_id;
        u64 content_storage_id;
        u64 unk[3];
    };

    struct save_create {
        u64 ivfc_sz;
        u64 unk;
        u64 ivfc_blocksz;
        u64 tid;
        u32 input_param;
        u8 save_storage;
        u8 save_type;
        std::array<u8, 0x1a> uninitialized_data;
    };
};

} // namespace FileSystem
} // namespace Service
