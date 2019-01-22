// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/frontend/scope_acquire_window_context.h"
#include "core/settings.h"
#include "video_core/gpu.h"
#include "video_core/gpu_thread.h"
#include "video_core/renderer_base.h"

namespace {

/// Executes a single GPU thread command
static void ExecuteCommand(VideoCore::CommandData* command, VideoCore::RendererBase& renderer,
                           Tegra::DmaPusher& dma_pusher) {
    if (const auto submit_list = std::get_if<VideoCore::SubmitListCommand>(command)) {
        dma_pusher.Push(std::move(submit_list->entries));
        dma_pusher.DispatchCalls();
    } else if (const auto data = std::get_if<VideoCore::SwapBuffersCommand>(command)) {
        renderer.SwapBuffers(data->framebuffer);
    } else if (const auto data = std::get_if<VideoCore::FlushRegionCommand>(command)) {
        renderer.Rasterizer().FlushRegion(data->addr, data->size);
    } else if (const auto data = std::get_if<VideoCore::InvalidateRegionCommand>(command)) {
        renderer.Rasterizer().InvalidateRegion(data->addr, data->size);
    } else if (const auto data = std::get_if<VideoCore::FlushAndInvalidateRegionCommand>(command)) {
        renderer.Rasterizer().FlushAndInvalidateRegion(data->addr, data->size);
    } else if (const auto data = std::get_if<VideoCore::WaitForIdleCommand>(command)) {
        // No-op command that is just used for synchronization
    } else {
        UNREACHABLE();
    }
}

/// Runs the GPU thread
static void RunThread(VideoCore::RendererBase& renderer, Tegra::DmaPusher& dma_pusher,
                      VideoCore::GPUThreadState& state) {

    Core::Frontend::ScopeAcquireWindowContext acquire_context{renderer.GetRenderWindow()};

    while (state.is_running) {
        {
            // Wait for CPU thread to send GPU commands
            std::unique_lock<std::mutex> lock{state.signal_mutex};
            state.signal_condition.wait(lock, [&] { return !state.IsIdle() || !state.is_running; });

            if (!state.is_running) {
                return;
            }

            for (auto& command : state.push_buffer) {
                state.pop_buffer.push(std::move(command));
            }
            state.push_buffer.clear();
        }

        // Execute all of the GPU commands
        while (!state.pop_buffer.empty()) {
            ExecuteCommand(state.pop_buffer.front().get(), renderer, dma_pusher);
            state.pop_buffer.pop();
        }

        {
            std::lock_guard<std::mutex> lock{state.idle_mutex};

            // Signal that the GPU thread has finished processing commands
            if (state.IsIdle()) {
                state.idle_condition.notify_one();
            }
        }
    }
}
} // Anonymous namespace

namespace VideoCore {

GPUThread::GPUThread(RendererBase& renderer, Tegra::DmaPusher& dma_pusher)
    : renderer{renderer}, dma_pusher{dma_pusher} {
    thread = std::make_unique<std::thread>(RunThread, std::ref(renderer), std::ref(dma_pusher),
                                           std::ref(state));
    thread_id = thread->get_id();
}

GPUThread::~GPUThread() {
    {
        // Notify GPU thread that a shutdown is pending
        std::lock_guard<std::mutex> lock{state.signal_mutex};
        state.is_running = false;
    }

    state.signal_condition.notify_one();
    thread->join();
}

void GPUThread::SubmitList(Tegra::CommandList&& entries) {
    if (entries.empty()) {
        return;
    }

    PushCommand(SubmitListCommand(std::move(entries)), false, false);
}

void GPUThread::SwapBuffers(
    std::optional<std::reference_wrapper<const Tegra::FramebufferConfig>> framebuffer) {
    PushCommand(SwapBuffersCommand(std::move(framebuffer)), true, false);
}

void GPUThread::FlushRegion(VAddr addr, u64 size) {
    if (Settings::values.use_accurate_gpu_emulation) {
        PushCommand(FlushRegionCommand(addr, size), true, false);
    } else {
        WaitForIdle();
    }
}

void GPUThread::InvalidateRegion(VAddr addr, u64 size) {
    PushCommand(InvalidateRegionCommand(addr, size), true, true);
}

void GPUThread::FlushAndInvalidateRegion(VAddr addr, u64 size) {
    if (Settings::values.use_accurate_gpu_emulation) {
        PushCommand(FlushAndInvalidateRegionCommand(addr, size), true, false);
    } else {
        InvalidateRegion(addr, size);
    }
}

void GPUThread::WaitForIdle() {
    PushCommand(WaitForIdleCommand(), true, true);
}

void GPUThread::PushCommand(CommandData&& command_data, bool wait_for_idle, bool allow_on_cpu) {
    if ((allow_on_cpu && state.IsIdle()) || IsGpuThread()) {
        // Execute the command synchronously on the current thread
        ExecuteCommand(&command_data, renderer, dma_pusher);
        return;
    }

    {
        // Push the command to the GPU thread
        std::lock_guard<std::mutex> lock{state.signal_mutex};
        state.push_buffer.push_back(std::move(std::make_unique<CommandData>(command_data)));
    }

    // Signal the GPU thread that commands are pending
    state.signal_condition.notify_one();

    if (wait_for_idle) {
        // Wait for the GPU to be idle (all commands to be executed)
        std::unique_lock<std::mutex> lock{state.idle_mutex};
        state.idle_condition.wait(lock, [this] { return state.IsIdle(); });
    }
}

} // namespace VideoCore
