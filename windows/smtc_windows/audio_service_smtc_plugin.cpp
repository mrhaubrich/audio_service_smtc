#include "audio_service_smtc_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

namespace audio_service_smtc {

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
    : registrar_(registrar),
      smtc_implementation_(std::make_unique<SmtcWindows>()) {}

AudioServiceSmtcPlugin::~AudioServiceSmtcPlugin() {}

void AudioServiceSmtcPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (method_call.method_name().compare("initialize") == 0) {
    // Get app identity from arguments if provided
    std::string identity = "audio_service_smtc";
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (arguments) {
      auto it = arguments->find(flutter::EncodableValue("identity"));
      if (it != arguments->end()) {
        const auto* identity_value = std::get_if<std::string>(&it->second);
        if (identity_value) {
          identity = *identity_value;
        }
      }
    }
    
    bool success = smtc_implementation_->Initialize(identity);
    result->Success(flutter::EncodableValue(success));
  } 
  else if (method_call.method_name().compare("updatePlaybackStatus") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (arguments) {
      auto it = arguments->find(flutter::EncodableValue("status"));
      if (it != arguments->end() && std::holds_alternative<std::string>(it->second)) {
        std::string status = std::get<std::string>(it->second);
        bool success = smtc_implementation_->UpdatePlaybackStatus(status);
        result->Success(flutter::EncodableValue(success));
      } else {
        result->Error("invalid_arguments", "Status argument is required");
      }
    } else {
      result->Error("invalid_arguments", "Arguments required");
    }
  } 
  else if (method_call.method_name().compare("updateMetadata") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (arguments) {
      std::string title, artist, album, albumArtUrl;
      int64_t duration = 0;
      
      // Extract title (required)
      auto title_it = arguments->find(flutter::EncodableValue("title"));
      if (title_it != arguments->end() && std::holds_alternative<std::string>(title_it->second)) {
        title = std::get<std::string>(title_it->second);
      } else {
        result->Error("invalid_arguments", "Title is required");
        return;
      }
      
      // Extract optional fields
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
      
      auto art_it = arguments->find(flutter::EncodableValue("albumArtUrl"));
      if (art_it != arguments->end() && std::holds_alternative<std::string>(art_it->second)) {
        albumArtUrl = std::get<std::string>(art_it->second);
      }
      
      bool success = smtc_implementation_->UpdateMetadata(title, artist, album, duration, albumArtUrl);
      result->Success(flutter::EncodableValue(success));
    } else {
      result->Error("invalid_arguments", "Arguments required");
    }
  } 
  else if (method_call.method_name().compare("setCallbacks") == 0) {
    // Set control callback to send events back to Flutter
    smtc_implementation_->SetControlCallback(
      [&](const std::string& command) {
        // Here you would implement a mechanism to invoke methods on Flutter side
        // For now, just log the command
        OutputDebugStringA(("SMTC command received: " + command).c_str());
      }
    );
    
    result->Success(flutter::EncodableValue(true));
  } 
  else {
    result->NotImplemented();
  }
}

}  // namespace audio_service_smtc

// This registers the plugin with the Flutter engine
void AudioServiceSmtcPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  audio_service_smtc::AudioServiceSmtcPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
