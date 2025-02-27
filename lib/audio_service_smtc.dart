/// A Windows implementation of audio service using System Media Transport Controls.
library audio_service_smtc;

import 'package:audio_service_smtc/src/audio_service_smtc.dart';

export 'src/audio_handler_callbacks.dart';
export 'src/audio_service_platform.dart';
export 'src/metadata.dart';
export 'src/smtc_plugin.dart';

/// Registers the Windows implementation of the audio service plugin.
///
/// This method should be called from your Flutter app's main method:
/// ```dart
/// void main() {
///   AudioServiceSmtc.registerWith();
///   runApp(MyApp());
/// }
/// ```
class AudioServiceSmtc {
  /// Register this implementation as the default instance for Windows platform
  static void registerWith() {
    AudioServiceSmtcImpl.registerWith();
  }
}
