// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <variant>

#include "video_core/dma_pusher.h"

namespace Tegra {
struct FramebufferConfig;
}

namespace VideoCore {

class RendererBase;

struct SubmitListCommand;
struct SwapBuffersCommand;
struct FlushRegionCommand;
struct InvalidateRegionCommand;
struct FlushAndInvalidateRegionCommand;
struct WaitForIdleCommand;

using CommandData =
    std::variant<SubmitListCommand, SwapBuffersCommand, FlushRegionCommand, InvalidateRegionCommand,
                 FlushAndInvalidateRegionCommand, WaitForIdleCommand>;

struct GPUThreadState final {
    std::atomic<bool> is_running{true};
    std::condition_variable signal_condition;
    std::mutex signal_mutex;
    std::condition_variable idle_condition;
    std::mutex idle_mutex;
    std::vector<std::unique_ptr<CommandData>> push_buffer;
    std::queue<std::unique_ptr<CommandData>> pop_buffer;

    bool IsIdle() const {
        return push_buffer.empty() && pop_buffer.empty();
    }
};

struct SubmitListCommand final {
    explicit SubmitListCommand(Tegra::CommandList&& entries) : entries{std::move(entries)} {}

    Tegra::CommandList entries;
};

struct SwapBuffersCommand final {
    explicit SwapBuffersCommand(std::optional<const Tegra::FramebufferConfig> framebuffer)
        : framebuffer{std::move(framebuffer)} {}

    std::optional<const Tegra::FramebufferConfig> framebuffer;
};

struct FlushRegionCommand final {
    explicit constexpr FlushRegionCommand(VAddr addr, u64 size) : addr{addr}, size{size} {}

    const VAddr addr;
    const u64 size;
};

struct InvalidateRegionCommand final {
    explicit constexpr InvalidateRegionCommand(VAddr addr, u64 size) : addr{addr}, size{size} {}

    const VAddr addr;
    const u64 size;
};

struct FlushAndInvalidateRegionCommand final {
    explicit constexpr FlushAndInvalidateRegionCommand(VAddr addr, u64 size)
        : addr{addr}, size{size} {}

    const VAddr addr;
    const u64 size;
};

struct WaitForIdleCommand final {
    explicit constexpr WaitForIdleCommand() {}
};

class GPUThread final {
public:
    explicit GPUThread(RendererBase& renderer, Tegra::DmaPusher& dma_pusher);
    ~GPUThread();

    /// Push GPU command entries to be processed
    void SubmitList(Tegra::CommandList&& entries);

    /// Swap buffers (render frame)
    void SwapBuffers(
        std::optional<std::reference_wrapper<const Tegra::FramebufferConfig>> framebuffer);

    /// Notify rasterizer that any caches of the specified region should be flushed to Switch memory
    void FlushRegion(VAddr addr, u64 size);

    /// Notify rasterizer that any caches of the specified region should be invalidated
    void InvalidateRegion(VAddr addr, u64 size);

    /// Notify rasterizer that any caches of the specified region should be flushed and invalidated
    void FlushAndInvalidateRegion(VAddr addr, u64 size);

    /// Waits the caller until the GPU thread is idle, used for synchronization
    void WaitForIdle();

private:
    /// Pushes a command to be executed by the GPU thread
    void PushCommand(CommandData&& command_data, bool wait_for_idle, bool allow_on_cpu);

    /// Returns true if this is called by the GPU thread
    bool IsGpuThread() const {
        return std::this_thread::get_id() == thread_id;
    }

private:
    GPUThreadState state;
    std::unique_ptr<std::thread> thread;
    std::thread::id thread_id{};
    RendererBase& renderer;
    Tegra::DmaPusher& dma_pusher;
};

} // namespace VideoCore
