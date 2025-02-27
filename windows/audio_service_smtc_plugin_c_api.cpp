#include "include/audio_service_smtc/audio_service_smtc_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "smtc_windows/audio_service_smtc_plugin.h"

void AudioServiceSmtcPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  audio_service_smtc::AudioServiceSmtcPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
