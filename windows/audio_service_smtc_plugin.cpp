#include "audio_service_smtc_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>
#include <map>

#include "smtc_windows/smtc_windows.h"

namespace audio_service_smtc {

class AudioServiceSmtcPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  AudioServiceSmtcPlugin(flutter::PluginRegistrarWindows *registrar);

  virtual ~AudioServiceSmtcPlugin();

 private:
  // The registrar for this plugin, for accessing the window and sending events.
  flutter::PluginRegistrarWindows *registrar_;

  // SMTC handler pointer
  void* smtcHandler_ = nullptr;

  // Method channel for callbacks
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
      
  // Callback for SMTC control events
  void OnControlEvent(const char* controlType);
  
  // Callback for SMTC position changes
  void OnPositionChange(int64_t positionInMicroseconds);
};

// static
void AudioServiceSmtcPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "audio_service_smtc",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<AudioServiceSmtcPlugin>(registrar);

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

AudioServiceSmtcPlugin::AudioServiceSmtcPlugin(flutter::PluginRegistrarWindows *registrar)
    : registrar_(registrar) {
  // Create the method channel for callbacks
  channel_ =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar_->messenger(), "audio_service_smtc/events",
          &flutter::StandardMethodCodec::GetInstance());
}

AudioServiceSmtcPlugin::~AudioServiceSmtcPlugin() {
  if (smtcHandler_) {
    DisposeSMTCHandler(smtcHandler_);
    smtcHandler_ = nullptr;
  }
}

void AudioServiceSmtcPlugin::OnControlEvent(const char* controlType) {
  if (!controlType) return;
  
  // Dispatch the control event back to Flutter
  flutter::EncodableMap args;
  args[flutter::EncodableValue("type")] = flutter::EncodableValue("control");
  args[flutter::EncodableValue("controlType")] = flutter::EncodableValue(std::string(controlType));
  
  registrar_->messenger()->Send(
    "audio_service_smtc/events", 
    flutter::StandardMethodCodec::GetInstance()->EncodeSuccessEnvelope(flutter::EncodableValue(args))
  );
}

void AudioServiceSmtcPlugin::OnPositionChange(int64_t positionInMicroseconds) {
  // Dispatch the position change event back to Flutter
  flutter::EncodableMap args;
  args[flutter::EncodableValue("type")] = flutter::EncodableValue("position");
  args[flutter::EncodableValue("position")] = flutter::EncodableValue(positionInMicroseconds);
  
  registrar_->messenger()->Send(
    "audio_service_smtc/events", 
    flutter::StandardMethodCodec::GetInstance()->EncodeSuccessEnvelope(flutter::EncodableValue(args))
  );
}

void AudioServiceSmtcPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  // Initialize SMTC
  if (method_call.method_name() == "initialize") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("Invalid arguments", "Expected a map");
      return;
    }
    
    auto identity_it = arguments->find(flutter::EncodableValue("identity"));
    if (identity_it == arguments->end() || !std::holds_alternative<std::string>(identity_it->second)) {
      result->Error("Invalid arguments", "Identity must be a string");
      return;
    }
    
    std::string identity = std::get<std::string>(identity_it->second);
    
    // Clean up previous handler if exists
    if (smtcHandler_) {
      DisposeSMTCHandler(smtcHandler_);
    }
    
    // Create new handler
    smtcHandler_ = CreateSMTCHandler(identity.c_str());
    
    if (!smtcHandler_) {
      result->Error("Initialization failed", "Failed to create SMTC handler");
      return;
    }
    
    // Set up callbacks
    SetCallbacks(
      smtcHandler_,
      [this](const char* controlType) { this->OnControlEvent(controlType); },
      [this](int64_t position) { this->OnPositionChange(position); }
    );
    
    result->Success();
    return;
  }
  
  // Update playback status
  else if (method_call.method_name() == "updatePlaybackStatus") {
    if (!smtcHandler_) {
      result->Error("Not initialized", "SMTC handler not initialized");
      return;
    }
    
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("Invalid arguments", "Expected a map");
      return;
    }
    
    auto status_it = arguments->find(flutter::EncodableValue("status"));
    if (status_it == arguments->end() || !std::holds_alternative<std::string>(status_it->second)) {
      result->Error("Invalid arguments", "Status must be a string");
      return;
    }
    
    std::string status = std::get<std::string>(status_it->second);
    UpdatePlaybackStatus(smtcHandler_, status.c_str());
    
    result->Success();
    return;
  }
  
  // Update metadata
  else if (method_call.method_name() == "updateMetadata") {
    if (!smtcHandler_) {
      result->Error("Not initialized", "SMTC handler not initialized");
      return;
    }
    
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("Invalid arguments", "Expected a map");
      return;
    }
    
    // Extract metadata fields
    std::string title;
    std::string artist;
    std::string album;
    int64_t duration = 0;
    std::string albumArtUrl;
    
    auto title_it = arguments->find(flutter::EncodableValue("title"));
    if (title_it != arguments->end() && std::holds_alternative<std::string>(title_it->second)) {
      title = std::get<std::string>(title_it->second);
    }
    
    auto artist_it = arguments->find(flutter::EncodableValue("artist"));
    if (artist_it != arguments->end() && std::holds_alternative<std::string>(artist_it->second)) {
      artist = std::get<std::string>(artist_it->second);
    }
    
    auto album_it = arguments->find(flutter::EncodableValue("album"));
    if (album_it != arguments->end() && std::holds_alternative<std::string>(album_it->second)) {
      album = std::get<std::string>(album_it->second);
    }
    
    auto duration_it = arguments->find(flutter::EncodableValue("duration"));
    if (duration_it != arguments->end() && std::holds_alternative<int64_t>(duration_it->second)) {
      duration = std::get<int64_t>(duration_it->second);
    }
    
    auto albumArtUrl_it = arguments->find(flutter::EncodableValue("albumArtUrl"));
    if (albumArtUrl_it != arguments->end() && std::holds_alternative<std::string>(albumArtUrl_it->second)) {
      albumArtUrl = std::get<std::string>(albumArtUrl_it->second);
    }
    
    UpdateMetadata(smtcHandler_, title.c_str(), artist.c_str(), album.c_str(), duration, albumArtUrl.c_str());
    
    result->Success();
    return;
  }
  
  // Dispose the SMTC handler
  else if (method_call.method_name() == "dispose") {
    if (smtcHandler_) {
      DisposeSMTCHandler(smtcHandler_);
      smtcHandler_ = nullptr;
    }
    
    result->Success();
    return;
  }
  
  else {
    result->NotImplemented();
    return;
  }
}

}  // namespace audio_service_smtc

void AudioServiceSmtcPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  audio_service_smtc::AudioServiceSmtcPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
