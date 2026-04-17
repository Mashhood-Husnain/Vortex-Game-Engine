#include "util/vortex_nvml.hpp"
#include "util/vortex_logs.hpp"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define LOAD_LIBRARY(name) LoadLibraryA(name)
    #define GET_PROC(lib, name) GetProcAddress((HMODULE)lib, name)
    #define FREE_LIBRARY(lib) FreeLibrary((HMODULE)lib)
    const char* NVML_LIB_NAME = "nvml.dll";
#else
    #include <dlfcn.h>
    #define LOAD_LIBRARY(name) dlopen(name, RTLD_NOW)
    #define GET_PROC(lib, name) dlsym(lib, name)
    #define FREE_LIBRARY(lib) dlclose(lib)
    const char* NVML_LIB_NAME = "libnvidia-ml.so.1";
#endif

void* VortexNVML::lib_handle = nullptr;
bool VortexNVML::is_initialized = false;
nvmlDevice_t VortexNVML::device = nullptr;

VortexNVML::nvmlInit_t VortexNVML::ptr_nvmlInit = nullptr;
VortexNVML::nvmlShutdown_t VortexNVML::ptr_nvmlShutdown = nullptr;
VortexNVML::nvmlDeviceGetHandleByIndex_t VortexNVML::ptr_nvmlDeviceGetHandleByIndex = nullptr;
VortexNVML::nvmlDeviceGetMemoryInfo_t VortexNVML::ptr_nvmlDeviceGetMemoryInfo = nullptr;
VortexNVML::nvmlDeviceGetUtilizationRates_t VortexNVML::ptr_nvmlDeviceGetUtilizationRates = nullptr;

bool VortexNVML::init()
{
    if (is_initialized) return true;

    lib_handle = (void*)LOAD_LIBRARY(NVML_LIB_NAME);
    if (!lib_handle)
    {
        VORTEX_WARN("[VortexNVML] NVIDIA Driver not found. Disabling GPU metrics.");
    
        return false;
    }

    ptr_nvmlInit = (nvmlInit_t)GET_PROC(lib_handle, "nvmlInit_v2");
    ptr_nvmlShutdown = (nvmlShutdown_t)GET_PROC(lib_handle, "nvmlShutdown");
    ptr_nvmlDeviceGetHandleByIndex = (nvmlDeviceGetHandleByIndex_t)GET_PROC(lib_handle, "nvmlDeviceGetHandleByIndex_v2");
    ptr_nvmlDeviceGetMemoryInfo = (nvmlDeviceGetMemoryInfo_t)GET_PROC(lib_handle, "nvmlDeviceGetMemoryInfo");
    ptr_nvmlDeviceGetUtilizationRates = (nvmlDeviceGetUtilizationRates_t)GET_PROC(lib_handle, "nvmlDeviceGetUtilizationRates");

    if (!ptr_nvmlInit || !ptr_nvmlDeviceGetHandleByIndex || !ptr_nvmlDeviceGetMemoryInfo || !ptr_nvmlDeviceGetUtilizationRates)
    {
        VORTEX_WARN("[VortexNVML] Failed to map NVML functions.");
        FREE_LIBRARY(lib_handle);
        return false;
    }

    if (ptr_nvmlInit() != NVML_SUCCESS) return false;
    if (ptr_nvmlDeviceGetHandleByIndex(0, &device) != NVML_SUCCESS) return false;

    is_initialized = true;
    return true;
}

GPUStats VortexNVML::get_stats()
{
    GPUStats stats;
    if (!is_initialized) return stats;

    nvmlUtilization_t utilization;
    if (ptr_nvmlDeviceGetUtilizationRates(device, &utilization) == NVML_SUCCESS)
    {
        stats.utilization = utilization.gpu;
    }

    nvmlMemory_t memory;
    if (ptr_nvmlDeviceGetMemoryInfo(device, &memory) == NVML_SUCCESS)
    {
        stats.mem_used_mb = memory.used / (1024 * 1024);
        stats.mem_total_mb = memory.total / (1024 * 1024);
    }

    stats.available = true;
    return stats;
}

void VortexNVML::shutdown()
{
    if (is_initialized && ptr_nvmlShutdown)
    {
        ptr_nvmlShutdown();
        is_initialized = false;
    }
    if (lib_handle)
    {
        FREE_LIBRARY(lib_handle);
        lib_handle = nullptr;
    }
}
