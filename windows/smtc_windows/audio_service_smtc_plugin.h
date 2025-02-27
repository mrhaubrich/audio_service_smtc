#ifndef FLUTTER_PLUGIN_AUDIO_SERVICE_SMTC_PLUGIN_H_
#define FLUTTER_PLUGIN_AUDIO_SERVICE_SMTC_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>
#include "smtc_windows.h"

namespace audio_service_smtc {

class AudioServiceSmtcPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  AudioServiceSmtcPlugin(flutter::PluginRegistrarWindows *registrar);
  virtual ~AudioServiceSmtcPlugin();

  // Disallow copy and assign.
  AudioServiceSmtcPlugin(const AudioServiceSmtcPlugin&) = delete;
  AudioServiceSmtcPlugin& operator=(const AudioServiceSmtcPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

 private:
  flutter::PluginRegistrarWindows *registrar_;
  std::unique_ptr<SmtcWindows> smtc_implementation_;
};

}  // namespace audio_service_smtc

#endif  // FLUTTER_PLUGIN_AUDIO_SERVICE_SMTC_PLUGIN_H_
