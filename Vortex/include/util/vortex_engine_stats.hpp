/*
 * File: vortex_engine_stats.hpp
 * Project: VortexEngine
 * Description: Monitor harware usage
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <unistd.h>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>
#include <nvml.h>

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
    }
};

// RAM usage in MB
inline float get_ram_usage_mb()
{
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
}

// Threaded engine stats using NVML
class EngineStatsThreaded
{
public:
    EngineStatsThreaded()
    {
        running = true;

        // Initialize NVML
        if (nvmlInit() != NVML_SUCCESS)
            nvml_available = false;

        stats_thread = std::thread([this]() { this->poll_gpu_loop(); });
    }

    ~EngineStatsThreaded()
    {
        running = false;
        if (stats_thread.joinable())
            stats_thread.join();

        if (nvml_available)
            nvmlShutdown();
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
    bool nvml_available = true;

    void poll_gpu_loop()
    {
        while (running)
        {
            EngineStats temp;
            if (nvml_available)
            {
                nvmlDevice_t device;
                unsigned int device_count = 0;

                if (nvmlDeviceGetCount(&device_count) == NVML_SUCCESS && device_count > 0)
                {
                    nvmlDeviceGetHandleByIndex(0, &device);

                    // GPU utilization
                    nvmlUtilization_t util;
                    if (nvmlDeviceGetUtilizationRates(device, &util) == NVML_SUCCESS)
                        temp.gpu_usage = static_cast<float>(util.gpu);

                    // GPU memory
                    nvmlMemory_t mem;
                    if (nvmlDeviceGetMemoryInfo(device, &mem) == NVML_SUCCESS)
                    {
                        temp.gpu_mem_usage = static_cast<float>(mem.used / (1024 * 1024));
                        temp.gpu_mem_total = static_cast<float>(mem.total / (1024 * 1024));
                    }
                }
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
