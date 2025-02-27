#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <string>
#include <functional>
#include <iostream>
#include <mutex>

using namespace winrt;
using namespace Windows::Media;
using namespace Windows::Foundation;

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

class SMTCHandler {
public:
    SMTCHandler(const std::string& identity) : _identity(identity) {
        init_apartment();
        InitializeControls();
    }

    ~SMTCHandler() {
        // Clean up resources
        try {
            if (_controls) {
                _controls.ButtonPressed(_buttonPressedToken);
                _controls = nullptr;
            }
            _displayUpdater = nullptr;
        }
        catch (...) {
            // Ignore exceptions during cleanup
        }
    }

    void UpdatePlaybackState(const std::string& state) {
        if (!_controls) return;

        MediaPlaybackStatus status = MediaPlaybackStatus::Closed;
        
        if (state == "Playing") {
            status = MediaPlaybackStatus::Playing;
        } else if (state == "Paused") {
            status = MediaPlaybackStatus::Paused;
        } else if (state == "Stopped") {
            status = MediaPlaybackStatus::Stopped;
        }

        _controls.PlaybackStatus(status);
    }

    void UpdateMetadata(const std::string& title, const std::string& artist, 
                         const std::string& album, int64_t durationMicroseconds, 
                         const std::string& albumArtUrl) {
        if (!_displayUpdater) return;

        try {
            // Clear existing properties
            _displayUpdater.Type(MediaPlaybackType::Music);
            
            // Set music properties
            auto musicProps = _displayUpdater.MusicProperties();
            musicProps.Title(winrt::to_hstring(title));
            
            if (!artist.empty()) {
                musicProps.Artist(winrt::to_hstring(artist));
            }
            
            if (!album.empty()) {
                musicProps.AlbumTitle(winrt::to_hstring(album));
            }
            
            // Set thumbnail if URL is provided
            if (!albumArtUrl.empty()) {
                // Note: This is a simplified example. 
                // In a real implementation, you would need to download
                // the image or use a local file
                // _displayUpdater.Thumbnail(/* Set thumbnail from URL */);
            }
            
            // Apply changes
            _displayUpdater.Update();
        }
        catch (const winrt::hresult_error& ex) {
            std::cerr << "Error updating metadata: " << winrt::to_string(ex.message()) << std::endl;
        }
    }

    void SetCallbacks(std::function<void(const char*)> controlCb, 
                      std::function<void(int64_t)> positionCb) {
        _controlCallback = controlCb;
        _positionCallback = positionCb;
    }

private:
    std::string _identity;
    SystemMediaTransportControls _controls{ nullptr };
    SystemMediaTransportControlsDisplayUpdater _displayUpdater{ nullptr };
    winrt::event_token _buttonPressedToken{};
    
    std::function<void(const char*)> _controlCallback;
    std::function<void(int64_t)> _positionCallback;
    std::mutex _callbackMutex;

    void InitializeControls() {
        try {
            // Get the system media transport controls for the current view
            _controls = SystemMediaTransportControlsInterop::GetForCurrentView();
            
            if (!_controls) {
                std::cerr << "Failed to get SMTC for current view" << std::endl;
                return;
            }
            
            // Enable controls
            _controls.IsEnabled(true);
            _controls.IsPlayEnabled(true);
            _controls.IsPauseEnabled(true);
            _controls.IsNextEnabled(true);
            _controls.IsPreviousEnabled(true);
            _controls.IsStopEnabled(true);
            
            // Get the display updater
            _displayUpdater = _controls.DisplayUpdater();
            
            // Register for button events
            _buttonPressedToken = _controls.ButtonPressed([this](
                SystemMediaTransportControls const& sender,
                SystemMediaTransportControlsButtonPressedEventArgs const& args) {
                    
                std::string command;
                
                switch (args.Button()) {
                case SystemMediaTransportControlsButton::Play:
                    command = "play";
                    break;
                case SystemMediaTransportControlsButton::Pause:
                    command = "pause";
                    break;
                case SystemMediaTransportControlsButton::Next:
                    command = "next";
                    break;
                case SystemMediaTransportControlsButton::Previous:
                    command = "previous";
                    break;
                case SystemMediaTransportControlsButton::Stop:
                    command = "stop";
                    break;
                default:
                    return;
                }
                
                std::lock_guard<std::mutex> lock(_callbackMutex);
                if (_controlCallback) {
                    _controlCallback(command.c_str());
                }
            });
        }
        catch (const winrt::hresult_error& ex) {
            std::cerr << "Error initializing SMTC: " << winrt::to_string(ex.message()) << std::endl;
        }
    }
};

// Export implementation
void* CreateSMTCHandler(const char* identity) {
    try {
        return new SMTCHandler(identity);
    }
    catch (...) {
        return nullptr;
    }
}

void DisposeSMTCHandler(void* handler) {
    if (handler) {
        delete static_cast<SMTCHandler*>(handler);
    }
}

void UpdatePlaybackStatus(void* handler, const char* status) {
    if (handler && status) {
        static_cast<SMTCHandler*>(handler)->UpdatePlaybackState(status);
    }
}

void UpdateMetadata(void* handler, const char* title, const char* artist, 
                   const char* album, int64_t duration, const char* albumArtUrl) {
    if (handler && title) {
        static_cast<SMTCHandler*>(handler)->UpdateMetadata(
            title, 
            artist ? artist : "", 
            album ? album : "", 
            duration,
            albumArtUrl ? albumArtUrl : "");
    }
}

void SetCallbacks(void* handler, 
                 std::function<void(const char*)> controlCallback,
                 std::function<void(int64_t)> positionCallback) {
    if (handler) {
        static_cast<SMTCHandler*>(handler)->SetCallbacks(controlCallback, positionCallback);
    }
}
