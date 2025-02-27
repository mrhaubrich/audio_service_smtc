import 'dart:async';
import 'dart:io';

import 'package:audio_service_smtc/src/metadata.dart';
import 'package:flutter/services.dart';

/// Plugin for interacting with the Windows SMTC (System Media Transport Controls).
class SmtcPlugin {
  /// Creates a new instance of the SMTC plugin.
  SmtcPlugin({required String identity}) {
    if (Platform.isWindows) {
      _initialize(identity);
    }
  }

  final MethodChannel _channel = const MethodChannel('audio_service_smtc');
  final _controlStreamController = StreamController<String>.broadcast();
  final _positionStreamController = StreamController<Duration>.broadcast();

  /// Stream of control events from SMTC (play, pause, next, previous, stop).
  Stream<String> get controlStream => _controlStreamController.stream;

  /// Stream of position change events.
  Stream<Duration> get positionStream => _positionStreamController.stream;

  /// Initialize the plugin with the given identity.
  Future<void> _initialize(String identity) async {
    try {
      await _channel.invokeMethod('initialize', {'identity': identity});

      // Set up event channel for callbacks from native code
      const eventChannel = MethodChannel('audio_service_smtc/events');
      eventChannel.setMethodCallHandler(_handleMethodCall);
    } catch (e) {
      print('Error initializing SMTC plugin: $e');
    }
  }

  /// Update the playback status in SMTC.
  Future<void> updatePlaybackStatus(String status) async {
    if (!Platform.isWindows) return;

    try {
      await _channel.invokeMethod('updatePlaybackStatus', {'status': status});
    } catch (e) {
      print('Error updating playback status: $e');
    }
  }

  /// Update metadata in SMTC.
  Future<void> updateMetadata(SmtcMetadata metadata) async {
    if (!Platform.isWindows) return;

    try {
      await _channel.invokeMethod('updateMetadata', {
        'title': metadata.title,
        'artist': metadata.artist?.join(', '),
        'album': metadata.album,
        'duration': metadata.duration?.inMicroseconds,
        'albumArtUrl': metadata.albumArtUrl,
      });
    } catch (e) {
      print('Error updating metadata: $e');
    }
  }

  /// Clean up resources.
  Future<void> dispose() async {
    if (!Platform.isWindows) return;

    try {
      await _channel.invokeMethod('dispose');
      _controlStreamController.close();
      _positionStreamController.close();
    } catch (e) {
      print('Error disposing SMTC plugin: $e');
    }
  }

  /// Handle method calls from the native side.
  Future<void> _handleMethodCall(MethodCall call) async {
    final args = call.arguments as Map<dynamic, dynamic>;
    final type = args['type'] as String;

    if (type == 'control') {
      final controlType = args['controlType'] as String;
      _controlStreamController.add(controlType);
    } else if (type == 'position') {
      final positionMicroseconds = args['position'] as int;
      _positionStreamController
          .add(Duration(microseconds: positionMicroseconds));
    }
  }
}
