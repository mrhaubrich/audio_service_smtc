#pragma once

#include <functional>
#include <cstdint>

// C interface for Flutter plugin
extern "C" {
    __declspec(dllexport) void* CreateSMTCHandler(const char* identity);
    __declspec(dllexport) void DisposeSMTCHandler(void* handler);
    __declspec(dllexport) void UpdatePlaybackStatus(void* handler, const char* status);
    __declspec(dllexport) void UpdateMetadata(void* handler, const char* title, const char* artist, 
        const char* album, int64_t duration, const char* albumArtUrl);
    __declspec(dllexport) void SetCallbacks(void* handler, 
        std::function<void(const char*)> controlCallback,
        std::function<void(int64_t)> positionCallback);
}
