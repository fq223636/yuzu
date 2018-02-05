// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/logging/log.h"
#include "core/core.h"
#include "core/file_sys/filesystem.h"
#include "core/file_sys/storage.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/client_port.h"
#include "core/hle/kernel/client_session.h"
#include "core/hle/service/filesystem/filesystem.h"
#include "core/hle/service/filesystem/fsp_srv.h"

namespace Service {
namespace FileSystem {

class IFile final : public ServiceFramework<IFile> {
public:
    IFile(std::string path, u32 mode) : ServiceFramework("IFile") {
        std::string openModes[] = {"", "rb", "wb+", "wb+", "ab+", "ab+", "ab+", "ab+"};
        fp = fopen(path.c_str(), openModes[mode].c_str());
        static const FunctionInfo functions[] = {
            {0, &IFile::Read, "Read"},
        };
        RegisterHandlers(functions);
    }

private:
    FILE* fp = NULL;
    void Read(Kernel::HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        u64 unk = rp.Pop<u64>();
        u64 offset = rp.Pop<u64>();
        u32 sz = rp.Pop<u32>();
        LOG_DEBUG(Service_FS, "called, unk=%016llx, offset=%016llx, sz=%x", unk, offset, sz);
        fseek(fp, offset, SEEK_SET);

        auto output_buffer = ctx.BufferDescriptorB()[0];
        std::vector<u8> output(output_buffer.Size());
        fread(&output[0], sz, 1, fp);
        Memory::WriteBlock(output_buffer.Address(), output.data(), output_buffer.Size());

        IPC::ResponseBuilder rb{ctx, 3};
        rb.Push(RESULT_SUCCESS);
        rb.Push<u32>(sz);

        // UNIMPLEMENTED();
    }
};

class IFileSystem final : public ServiceFramework<IFileSystem> {
public:
    IFileSystem() : ServiceFramework("IFileSystem") {
        static const FunctionInfo functions[] = {
            {8, &IFileSystem::OpenFile, "OpenFile"},
        };
        RegisterHandlers(functions);
    }

private:
    std::string base_path = "/SaveData";

    void OpenFile(Kernel::HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        u32 mode = rp.Pop<u32>();
        auto pathBuf = ctx.BufferDescriptorX()[0];
        std::vector<u8> input(pathBuf.size);
        Memory::ReadBlock(pathBuf.Address(), input.data(), pathBuf.size);
        std::string fullPath = base_path + std::string((char*)input.data());
        LOG_DEBUG(Service_FS, "called, mode=0x%x, path=%s", mode, fullPath.c_str());
        if (FileExists(fullPath)) {
            IPC::ResponseBuilder rb{ctx, 2, 0, 1};
            rb.Push(RESULT_SUCCESS);
            rb.PushIpcInterface<IFile>(fullPath, mode);
        } else {
            LOG_ERROR(Service_FS, "File does not exist!");
            IPC::ResponseBuilder rb{ctx, 2};
            rb.Push(ResultCode(0x2EE402));
        }
        // UNIMPLEMENTED();
    }

    bool FileExists(std::string path) {
        FILE* fp = fopen(path.c_str(), "rb");
        bool exists = false;
        if (fp) {
            exists = true;
            fclose(fp);
        }
        return exists;
    }
};

class IStorage final : public ServiceFramework<IStorage> {
public:
    IStorage(std::unique_ptr<FileSys::StorageBackend>&& backend)
        : ServiceFramework("IStorage"), backend(std::move(backend)) {
        static const FunctionInfo functions[] = {
            {0, &IStorage::Read, "Read"}, {1, nullptr, "Write"},   {2, nullptr, "Flush"},
            {3, nullptr, "SetSize"},      {4, nullptr, "GetSize"},
        };
        RegisterHandlers(functions);
    }

private:
    std::unique_ptr<FileSys::StorageBackend> backend;

    void Read(Kernel::HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        const s64 offset = rp.Pop<s64>();
        const s64 length = rp.Pop<s64>();
        const auto& descriptor = ctx.BufferDescriptorB()[0];

        LOG_DEBUG(Service_FS, "called, offset=0x%llx, length=0x%llx", offset, length);

        // Error checking
        ASSERT_MSG(length == descriptor.Size(), "unexpected size difference");
        if (length < 0) {
            IPC::ResponseBuilder rb{ctx, 2};
            rb.Push(ResultCode(ErrorModule::FS, ErrorDescription::InvalidLength));
            return;
        }
        if (offset < 0) {
            IPC::ResponseBuilder rb{ctx, 2};
            rb.Push(ResultCode(ErrorModule::FS, ErrorDescription::InvalidOffset));
            return;
        }

        // Read the data from the Storage backend
        std::vector<u8> output(length);
        ResultVal<size_t> res = backend->Read(offset, length, output.data());
        if (res.Failed()) {
            IPC::ResponseBuilder rb{ctx, 2};
            rb.Push(res.Code());
            return;
        }

        // Write the data to memory
        Memory::WriteBlock(descriptor.Address(), output.data(), descriptor.Size());

        IPC::ResponseBuilder rb{ctx, 2};
        rb.Push(RESULT_SUCCESS);
    }
};

FSP_SRV::FSP_SRV() : ServiceFramework("fsp-srv") {
    static const FunctionInfo functions[] = {
        {1, &FSP_SRV::Initalize, "Initalize"},
        {22, &FSP_SRV::CreateSaveData, "CreateSaveData"},
        {51, &FSP_SRV::MountSaveData, "MountSaveData"},
        {200, &FSP_SRV::OpenDataStorageByCurrentProcess, "OpenDataStorageByCurrentProcess"},
        {203, &FSP_SRV::OpenRomStorage, "OpenRomStorage"},
        {1005, &FSP_SRV::GetGlobalAccessLogMode, "GetGlobalAccessLogMode"},
    };
    RegisterHandlers(functions);
}

void FSP_SRV::TryLoadRomFS() {
    if (romfs) {
        return;
    }
    FileSys::Path unused;
    auto res = OpenFileSystem(Type::RomFS, unused);
    if (res.Succeeded()) {
        romfs = std::move(res.Unwrap());
    }
}

void FSP_SRV::Initalize(Kernel::HLERequestContext& ctx) {
    LOG_WARNING(Service_FS, "(STUBBED) called");

    IPC::ResponseBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
}

void FSP_SRV::GetGlobalAccessLogMode(Kernel::HLERequestContext& ctx) {
    LOG_WARNING(Service_FS, "(STUBBED) called");

    IPC::ResponseBuilder rb{ctx, 3};
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(5);
}

void FSP_SRV::OpenDataStorageByCurrentProcess(Kernel::HLERequestContext& ctx) {
    LOG_DEBUG(Service_FS, "called");

    TryLoadRomFS();
    if (!romfs) {
        // TODO (bunnei): Find the right error code to use here
        LOG_CRITICAL(Service_FS, "no file system interface available!");
        IPC::ResponseBuilder rb{ctx, 2};
        rb.Push(ResultCode(-1));
        return;
    }

    // Attempt to open a StorageBackend interface to the RomFS
    auto storage = romfs->OpenFile({}, {});
    if (storage.Failed()) {
        LOG_CRITICAL(Service_FS, "no storage interface available!");
        IPC::ResponseBuilder rb{ctx, 2};
        rb.Push(storage.Code());
        return;
    }

    IPC::ResponseBuilder rb{ctx, 2, 0, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushIpcInterface<IStorage>(std::move(storage.Unwrap()));
}

void FSP_SRV::CreateSaveData(Kernel::HLERequestContext& ctx) {
    LOG_DEBUG(Service_FS, "called");
    save_struct ss;
    save_create sc;
    IPC::RequestParser rp{ctx};
    ss = rp.PopRaw<save_struct>();
    sc = rp.PopRaw<save_create>();

    LOG_WARNING(
        Service_FS,
        "save_struct tid=%016llx, user_id_low=%016llx, user_id_high=%016llx, save_id=%016llx, "
        "content_storage_id=%016llx",
        ss.tid, ss.user_id[0], ss.user_id[1], ss.save_id, ss.content_storage_id);

    LOG_WARNING(Service_FS,
                "ivfc_sz=%016llx, unk=%016llx, ivfc_block_size=%016llx, tid=%016llx, "
                "input_param=%08x, save_storage=%02x, save_type=%02x",
                sc.ivfc_sz, sc.unk, sc.ivfc_blocksz, sc.tid, sc.input_param, sc.save_storage,
                sc.save_type);

    IPC::ResponseBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
}

void FSP_SRV::MountSaveData(Kernel::HLERequestContext& ctx) {
    LOG_DEBUG(Service_FS, "called");
    save_struct ss;
    IPC::RequestParser rp{ctx};
    u32 input = rp.Pop<u32>();
    ss = rp.PopRaw<save_struct>();
    LOG_WARNING(Service_FS,
                "input=%x, save_struct tid=%016llx, user_id_low=%016llx, user_id_high=%016llx, "
                "save_id=%016llx, "
                "content_storage_id=%016llx",
                input, ss.tid, ss.user_id[0], ss.user_id[1], ss.save_id, ss.content_storage_id);
    IPC::ResponseBuilder rb{ctx, 2, 0, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushIpcInterface<IFileSystem>();
}

void FSP_SRV::OpenRomStorage(Kernel::HLERequestContext& ctx) {
    LOG_WARNING(Service_FS, "(STUBBED) called, using OpenDataStorageByCurrentProcess");
    OpenDataStorageByCurrentProcess(ctx);
}

} // namespace FileSystem
} // namespace Service
