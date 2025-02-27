import 'dart:async';
import 'dart:developer';
import 'dart:io';

import 'package:audio_service_smtc/src/audio_handler_callbacks.dart';
import 'package:audio_service_smtc/src/audio_service_platform.dart';
import 'package:audio_service_smtc/src/metadata.dart';
import 'package:audio_service_smtc/src/smtc_plugin.dart';

/// {@template audio_service_smtc}
/// Windows implementation for audio service using System Media Transport Controls.
/// {@endtemplate}
class AudioServiceSmtcImpl implements AudioServicePlatform {
  /// {@macro audio_service_smtc}
  AudioServiceSmtcImpl();

  SmtcPlugin? _smtcPlugin;
  AudioHandlerCallbacks? _handlerCallbacks;
  bool _isPlaying = false;

  /// Register this implementation as the default instance
  static void registerWith() {
    if (Platform.isWindows) {
      AudioServicePlatform.instance = AudioServiceSmtcImpl();
    }
  }

  void _listenToControlStream() {
    if (_smtcPlugin == null) return;

    _smtcPlugin!.controlStream.listen((event) {
      log('Requested from SMTC: $event', name: 'audio_service_smtc');
      if (_handlerCallbacks == null) return;

      switch (event) {
        case 'play':
          _handlerCallbacks!.play(const PlayRequest());
        case 'pause':
          _handlerCallbacks!.pause(const PauseRequest());
        case 'next':
          _handlerCallbacks!.skipToNext(const SkipToNextRequest());
        case 'previous':
          _handlerCallbacks!.skipToPrevious(const SkipToPreviousRequest());
        case 'stop':
          _handlerCallbacks!.stop(const StopRequest());
      }
    });
  }

  void _listenToPositionStream() {
    if (_smtcPlugin == null) return;

    _smtcPlugin!.positionStream.listen((position) {
      if (_handlerCallbacks == null) return;
      _handlerCallbacks!.seek(SeekRequest(position: position));
    });
  }

  @override
  Future<void> configure(ConfigureRequest request) async {
    if (!Platform.isWindows) {
      log('SMTC is only available on Windows', name: 'audio_service_smtc');
      return;
    }

    log('Configure AudioServiceSmtc', name: 'audio_service_smtc');

    _smtcPlugin = SmtcPlugin(
      identity: request.config.androidNotificationChannelName ?? 'Audio Player',
    );

    _listenToControlStream();
    _listenToPositionStream();
  }

  @override
  Future<void> setState(SetStateRequest request) async {
    if (_smtcPlugin == null) return;

    _isPlaying = request.state.playing;
    final status = _isPlaying ? 'Playing' : 'Paused';
    await _smtcPlugin!.updatePlaybackStatus(status);
  }

  @override
  Future<void> setMediaItem(SetMediaItemRequest request) async {
    if (_smtcPlugin == null) return;

    List<String>? artist;
    if (request.mediaItem.artist != null) artist = [request.mediaItem.artist!];

    List<String>? genre;
    if (request.mediaItem.genre != null) genre = [request.mediaItem.genre!];

    final metadata = SmtcMetadata(
      title: request.mediaItem.title,
      artist: artist,
      album: request.mediaItem.album,
      duration: request.mediaItem.duration,
      albumArtUrl: request.mediaItem.artUri?.toString(),
      genre: genre,
    );

    await _smtcPlugin!.updateMetadata(metadata);
  }

  @override
  Future<void> setQueue(SetQueueRequest request) async {
    // SMTC doesn't support queue management
    log(
      'setQueue() has not been implemented for SMTC',
      name: 'audio_service_smtc',
    );
  }

  @override
  Future<void> stopService(StopServiceRequest request) async {
    if (_smtcPlugin == null) return;
    await _smtcPlugin!.updatePlaybackStatus('Stopped');
  }

  @override
  void setHandlerCallbacks(AudioHandlerCallbacks callbacks) {
    _handlerCallbacks = callbacks;
  }

  @override
  Future<void> notifyChildrenChanged(
    NotifyChildrenChangedRequest request,
  ) async {
    // Not supported in SMTC
    throw UnimplementedError(
      'notifyChildrenChanged() has not been implemented.',
    );
  }
}
