/*
 * File: vortex_engine_stats.hpp
 * Project: VortexEngine
 * Description: Monitor hardware usage (Cross-Platform)
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>

// Include our new Dynamic NVML loader instead of the static library
#include "util/vortex_nvml.hpp"

// OS-Specific includes for CPU and RAM polling
#if defined(__linux__)
    #include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <psapi.h>
#endif

struct EngineStats
{
    float cpu_usage = 0.0f;       // 0-100%
    float gpu_usage = -1.0f;      // -1 = unavailable
    float ram_usage = 0.0f;       // MB
    float gpu_mem_usage = 0.0f;   // MB
    float gpu_mem_total = 0.0f;   // MB
};

// Smooth CPU usage
class CPUUsageSmoothed
{
public:
    CPUUsageSmoothed(size_t sample_count = 10) : max_samples(sample_count) {}

    float get()
    {
        float current = get_cpu_raw();
        samples.push_back(current);
        if (samples.size() > max_samples)
            samples.erase(samples.begin());

        float sum = std::accumulate(samples.begin(), samples.end(), 0.0f);
        return sum / static_cast<float>(samples.size());
    }

private:
    std::vector<float> samples;
    size_t max_samples;

    float get_cpu_raw()
    {
#if defined(__linux__)
        // ==========================================
        // LINUX: /proc/stat Implementation
        // ==========================================
        static long last_total_time = 0;
        static long last_proc_time  = 0;

        std::ifstream stat_file("/proc/stat");
        std::string cpu;
        long user, nice, system, idle;
        stat_file >> cpu >> user >> nice >> system >> idle;
        long total_time = user + nice + system + idle;

        std::ifstream proc_file("/proc/self/stat");
        std::string tmp;
        long utime, stime;
        for (int i = 0; i < 13; i++) proc_file >> tmp;
        proc_file >> utime >> stime;
        long proc_time = utime + stime;

        long total_diff = total_time - last_total_time;
        long proc_diff  = proc_time  - last_proc_time;

        last_total_time = total_time;
        last_proc_time  = proc_time;

        if (total_diff == 0) return 0.0f;
        return 100.0f * static_cast<float>(proc_diff) / static_cast<float>(total_diff);

#elif defined(_WIN32) || defined(_WIN64)
        // ==========================================
        // WINDOWS: Win32 API Implementation
        // ==========================================
        static FILETIME prev_sys_kernel, prev_sys_user;
        static FILETIME prev_proc_kernel, prev_proc_user;
        static bool first_run = true;

        FILETIME sys_idle, sys_kernel, sys_user;
        FILETIME proc_creation, proc_exit, proc_kernel, proc_user;

        GetSystemTimes(&sys_idle, &sys_kernel, &sys_user);
        GetProcessTimes(GetCurrentProcess(), &proc_creation, &proc_exit, &proc_kernel, &proc_user);

        if (first_run) {
            prev_sys_kernel = sys_kernel;
            prev_sys_user = sys_user;
            prev_proc_kernel = proc_kernel;
            prev_proc_user = proc_user;
            first_run = false;
            return 0.0f;
        }

        auto ft_to_ull = [](const FILETIME& ft) {
            ULARGE_INTEGER uli;
            uli.LowPart = ft.dwLowDateTime;
            uli.HighPart = ft.dwHighDateTime;
            return uli.QuadPart;
        };

        ULONGLONG sys_diff = (ft_to_ull(sys_kernel) + ft_to_ull(sys_user)) -
                             (ft_to_ull(prev_sys_kernel) + ft_to_ull(prev_sys_user));

        ULONGLONG proc_diff = (ft_to_ull(proc_kernel) + ft_to_ull(proc_user)) -
                              (ft_to_ull(prev_proc_kernel) + ft_to_ull(prev_proc_user));

        prev_sys_kernel = sys_kernel;
        prev_sys_user = sys_user;
        prev_proc_kernel = proc_kernel;
        prev_proc_user = proc_user;

        if (sys_diff == 0) return 0.0f;
        return 100.0f * static_cast<float>(proc_diff) / static_cast<float>(sys_diff);
#endif
    }
};

// RAM usage in MB
inline float get_ram_usage_mb()
{
#if defined(__linux__)
    std::ifstream file("/proc/self/status");
    std::string line;
    while(std::getline(file, line))
    {
        if (line.rfind("VmRSS:", 0) == 0)
        {
            std::string value = line.substr(6);
            return std::stof(value) / 1024.0f; // KB -> MB
        }
    }
    return 0.0f;

#elif defined(_WIN32) || defined(_WIN64)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<float>(pmc.WorkingSetSize) / (1024.0f * 1024.0f); // Bytes -> MB
    }
    return 0.0f;
#endif
}

// Threaded engine stats using Dynamic NVML
class EngineStatsThreaded
{
public:
    EngineStatsThreaded()
    {
        running = true;

        // Initialize Dynamic NVML Wrapper
        VortexNVML::init();

        stats_thread = std::thread([this]() { this->poll_gpu_loop(); });
    }

    ~EngineStatsThreaded()
    {
        running = false;
        if (stats_thread.joinable())
            stats_thread.join();

        // Shutdown Dynamic NVML
        VortexNVML::shutdown();
    }

    EngineStats get_stats()
    {
        EngineStats s;
        s.cpu_usage = cpu_smoother.get();
        s.ram_usage = get_ram_usage_mb();

        // Copy GPU stats safely
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            s.gpu_usage      = cached_stats.gpu_usage;
            s.gpu_mem_usage  = cached_stats.gpu_mem_usage;
            s.gpu_mem_total  = cached_stats.gpu_mem_total;
        }

        return s;
    }

private:
    std::thread stats_thread;
    std::atomic<bool> running{true};
    EngineStats cached_stats;
    std::mutex stats_mutex;
    CPUUsageSmoothed cpu_smoother;

    void poll_gpu_loop()
    {
        while (running)
        {
            EngineStats temp;
            
            // Call our new Dynamic NVML Loader
            GPUStats gpu = VortexNVML::get_stats();

            if (gpu.available)
            {
                temp.gpu_usage = static_cast<float>(gpu.utilization);
                temp.gpu_mem_usage = static_cast<float>(gpu.mem_used_mb);
                temp.gpu_mem_total = static_cast<float>(gpu.mem_total_mb);
            }
            else
            {
                temp.gpu_usage = -1.0f; // Fallback for Intel/AMD/Mac
            }

            // Update shared stats
            {
                std::lock_guard<std::mutex> lock(stats_mutex);
                cached_stats = temp;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 2x/sec
        }
    }
};
