/*
 * File: vortex_nvml.hpp
 * Project: VortexEngine
 * Description: Dynamic runtime loader for NVIDIA Management Library
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <nvml.h>
#include <iostream>

struct GPUStats
{
    bool available = false;
    unsigned int utilization = 0;
    unsigned long long mem_used_mb = 0;
    unsigned long long mem_total_mb = 0;
};

class VortexNVML
{
private:
    static void* lib_handle;
    static bool is_initialized;
    static nvmlDevice_t device;

    typedef nvmlReturn_t (*nvmlInit_t)();
    typedef nvmlReturn_t (*nvmlShutdown_t)();
    typedef nvmlReturn_t (*nvmlDeviceGetHandleByIndex_t)(unsigned int, nvmlDevice_t *);
    typedef nvmlReturn_t (*nvmlDeviceGetMemoryInfo_t)(nvmlDevice_t, nvmlMemory_t *);
    typedef nvmlReturn_t (*nvmlDeviceGetUtilizationRates_t)(nvmlDevice_t, nvmlUtilization_t *);

    static nvmlInit_t ptr_nvmlInit;
    static nvmlShutdown_t ptr_nvmlShutdown;
    static nvmlDeviceGetHandleByIndex_t ptr_nvmlDeviceGetHandleByIndex;
    static nvmlDeviceGetMemoryInfo_t ptr_nvmlDeviceGetMemoryInfo;
    static nvmlDeviceGetUtilizationRates_t ptr_nvmlDeviceGetUtilizationRates;

public:
    static bool init();
    static void shutdown();
    static GPUStats get_stats();
};
