#include <windows.h>
// Add the following WinRT includes at the beginning
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.Collections.h>
#include "smtc_windows.h"
#include <string>
#include <functional>
#include <iostream>
#include <mutex>

using namespace winrt;
using namespace Windows::Media;
using namespace Windows::Foundation;

// Implementation class to hide WinRT details
class SMTCHandlerImpl {
public:
    SMTCHandlerImpl(const std::string& identity) : _identity(identity) {
        init_apartment();
        InitializeControls();
    }

    ~SMTCHandlerImpl() {
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

    void SetControlCallback(std::function<void(const std::string&)> controlCb) {
        _controlCallback = controlCb;
    }
    
    void SetPositionCallback(std::function<void(int64_t)> positionCb) {
        _positionCallback = positionCb;
    }

private:
    std::string _identity;
    SystemMediaTransportControls _controls{ nullptr };
    SystemMediaTransportControlsDisplayUpdater _displayUpdater{ nullptr };
    winrt::event_token _buttonPressedToken{};
    
    std::function<void(const std::string&)> _controlCallback;
    std::function<void(int64_t)> _positionCallback;
    std::mutex _callbackMutex;

    void InitializeControls() {
        try {
            // Get the system media transport controls for the current view
            _controls = SystemMediaTransportControls::GetForCurrentView();
            
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
                    _controlCallback(command);
                }
            });
        }
        catch (const winrt::hresult_error& ex) {
            std::cerr << "Error initializing SMTC: " << winrt::to_string(ex.message()) << std::endl;
        }
    }
};

// SmtcWindows implementation
SmtcWindows::SmtcWindows() : _initialized(false) {}

SmtcWindows::~SmtcWindows() {
    // Impl will be cleaned up by unique_ptr
}

bool SmtcWindows::Initialize(const std::string& identity) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_initialized) return true;
    
    try {
        _impl = std::make_unique<SMTCHandlerImpl>(identity);
        _initialized = true;
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to initialize SMTC: " << ex.what() << std::endl;
        return false;
    }
}

bool SmtcWindows::UpdatePlaybackStatus(const std::string& status) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_initialized || !_impl) return false;
    
    try {
        _impl->UpdatePlaybackState(status);
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error updating playback status: " << ex.what() << std::endl;
        return false;
    }
}

bool SmtcWindows::UpdateMetadata(const std::string& title, const std::string& artist, 
                               const std::string& album, int64_t duration, 
                               const std::string& albumArtUrl) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_initialized || !_impl) return false;
    
    try {
        _impl->UpdateMetadata(title, artist, album, duration, albumArtUrl);
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error updating metadata: " << ex.what() << std::endl;
        return false;
    }
}

void SmtcWindows::SetControlCallback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_initialized && _impl) {
        _impl->SetControlCallback(callback);
    }
}

void SmtcWindows::SetPositionCallback(std::function<void(int64_t)> callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_initialized && _impl) {
        _impl->SetPositionCallback(callback);
    }
}

// C interface implementation
void* CreateSMTCHandler(const char* identity) {
    try {
        return new SMTCHandlerImpl(identity ? identity : "audio_service_smtc");
    }
    catch (...) {
        return nullptr;
    }
}

void DisposeSMTCHandler(void* handler) {
    if (handler) {
        delete static_cast<SMTCHandlerImpl*>(handler);
    }
}

void UpdatePlaybackStatus(void* handler, const char* status) {
    if (handler && status) {
        static_cast<SMTCHandlerImpl*>(handler)->UpdatePlaybackState(status);
    }
}

void UpdateMetadata(void* handler, const char* title, const char* artist, 
                   const char* album, int64_t duration, const char* albumArtUrl) {
    if (handler && title) {
        static_cast<SMTCHandlerImpl*>(handler)->UpdateMetadata(
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
        auto impl = static_cast<SMTCHandlerImpl*>(handler);
        
        if (controlCallback) {
            impl->SetControlCallback([cb = controlCallback](const std::string& cmd) {
                cb(cmd.c_str());
            });
        }
        
        if (positionCallback) {
            impl->SetPositionCallback(positionCallback);
        }
    }
}
