#pragma once

#include <cstring>

inline void vortex_strncpy(char *dest, size_t dest_size, const char *src)
{
    if (!dest || dest_size == 0) return;

    #ifdef _WIN32
        strncpy_s(dest, dest_size, src, _TRUNCATE);
    #else
        strncpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    #endif
}
