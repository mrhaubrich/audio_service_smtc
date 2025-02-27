#pragma once

#include <functional>
#include <cstdint>
#include <string>
#include <memory>
#include <mutex>

// Forward declare handler class to avoid including WinRT headers in header
class SMTCHandlerImpl;

class SmtcWindows {
public:
    SmtcWindows();
    ~SmtcWindows();

    // Initialize the SMTC handler
    bool Initialize(const std::string& identity = "audio_service_smtc");
    
    // Update playback status (playing, paused, stopped)
    bool UpdatePlaybackStatus(const std::string& status);
    
    // Update media metadata
    bool UpdateMetadata(const std::string& title, const std::string& artist, 
                       const std::string& album, int64_t duration, 
                       const std::string& albumArtUrl);
    
    // Set callback for handling media controls from SMTC
    void SetControlCallback(std::function<void(const std::string&)> callback);
    
    // Set callback for position changes
    void SetPositionCallback(std::function<void(int64_t)> callback);

private:
    // Implementation using PIMPL to hide WinRT dependencies
    std::unique_ptr<SMTCHandlerImpl> _impl;
    std::mutex _mutex;
    bool _initialized;
};

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
