import 'package:audio_service_smtc/src/audio_handler_callbacks.dart';

/// Configuration request for the audio service
class ConfigureRequest {
  /// Creates a new configuration request
  const ConfigureRequest({required this.config});

  /// The configuration
  final AudioServiceConfig config;
}

/// Audio service configuration
class AudioServiceConfig {
  /// Creates a new audio service configuration
  const AudioServiceConfig({
    this.androidNotificationChannelId,
    this.androidNotificationChannelName,
    this.androidNotificationIcon,
    this.androidShowNotificationBadge,
    this.notificationColor,
    this.androidNotificationClickStartsActivity,
    this.androidNotificationOngoing,
    this.androidStopForegroundOnPause,
    this.artDownscaleWidth,
    this.artDownscaleHeight,
    this.fastForwardInterval,
    this.rewindInterval,
  });

  /// Android notification channel ID
  final String? androidNotificationChannelId;

  /// Android notification channel name
  final String? androidNotificationChannelName;

  /// Android notification icon resource
  final String? androidNotificationIcon;

  /// Whether to show a badge on the notification
  final bool? androidShowNotificationBadge;

  /// Color of the notification
  final int? notificationColor;

  /// Whether clicking the notification starts the activity
  final bool? androidNotificationClickStartsActivity;

  /// Whether the notification is ongoing
  final bool? androidNotificationOngoing;

  /// Whether to stop foreground service on pause
  final bool? androidStopForegroundOnPause;

  /// Width to downscale art to
  final int? artDownscaleWidth;

  /// Height to downscale art to
  final int? artDownscaleHeight;

  /// Fast forward interval
  final Duration? fastForwardInterval;

  /// Rewind interval
  final Duration? rewindInterval;
}

/// Request to set the state of the audio service
class SetStateRequest {
  /// Creates a new set state request
  const SetStateRequest({required this.state});

  /// The state to set
  final PlaybackState state;
}

/// Playback state
class PlaybackState {
  /// Creates a new playback state
  const PlaybackState({
    required this.playing,
    required this.updatePosition,
  });

  /// Whether playback is active
  final bool playing;

  /// Current playback position
  final Duration updatePosition;
}

/// Request to set the media item
class SetMediaItemRequest {
  /// Creates a new set media item request
  const SetMediaItemRequest({required this.mediaItem});

  /// The media item to set
  final MediaItem mediaItem;
}

/// A media item
class MediaItem {
  /// Creates a new media item
  const MediaItem({
    required this.id,
    required this.title,
    this.album,
    this.artist,
    this.genre,
    this.duration,
    this.artUri,
  });

  /// Unique identifier for the item
  final String id;

  /// Title of the item
  final String title;

  /// Album the item belongs to
  final String? album;

  /// Artist of the item
  final String? artist;

  /// Genre of the item
  final String? genre;

  /// Duration of the item
  final Duration? duration;

  /// URI to the item's art
  final Uri? artUri;
}

/// Request to set the queue
class SetQueueRequest {
  /// Creates a new set queue request
  const SetQueueRequest({required this.queue});

  /// The queue to set
  final List<MediaItem> queue;
}

/// Request to stop the service
class StopServiceRequest {
  /// Creates a new stop service request
  const StopServiceRequest();
}

/// Request to notify children changed
class NotifyChildrenChangedRequest {
  /// Creates a new notify children changed request
  const NotifyChildrenChangedRequest({required this.parentMediaId});

  /// ID of the parent media item
  final String parentMediaId;
}

/// Platform interface for audio service
abstract class AudioServicePlatform {
  /// The instance of the platform interface
  static AudioServicePlatform instance = _UnsupportedAudioServicePlatform();

  /// Configure the audio service
  Future<void> configure(ConfigureRequest request);

  /// Set the state of the audio service
  Future<void> setState(SetStateRequest request);

  /// Set the media item
  Future<void> setMediaItem(SetMediaItemRequest request);

  /// Set the queue
  Future<void> setQueue(SetQueueRequest request);

  /// Stop the service
  Future<void> stopService(StopServiceRequest request);

  /// Notify that children have changed
  Future<void> notifyChildrenChanged(NotifyChildrenChangedRequest request);

  /// Set handler callbacks
  void setHandlerCallbacks(AudioHandlerCallbacks callbacks);
}

/// Unsupported platform implementation
class _UnsupportedAudioServicePlatform implements AudioServicePlatform {
  @override
  Future<void> configure(ConfigureRequest request) {
    throw UnimplementedError('configure() is not supported on this platform');
  }

  @override
  Future<void> setState(SetStateRequest request) {
    throw UnimplementedError('setState() is not supported on this platform');
  }

  @override
  Future<void> setMediaItem(SetMediaItemRequest request) {
    throw UnimplementedError(
        'setMediaItem() is not supported on this platform');
  }

  @override
  Future<void> setQueue(SetQueueRequest request) {
    throw UnimplementedError('setQueue() is not supported on this platform');
  }

  @override
  Future<void> stopService(StopServiceRequest request) {
    throw UnimplementedError('stopService() is not supported on this platform');
  }

  @override
  Future<void> notifyChildrenChanged(NotifyChildrenChangedRequest request) {
    throw UnimplementedError(
        'notifyChildrenChanged() is not supported on this platform');
  }

  @override
  void setHandlerCallbacks(AudioHandlerCallbacks callbacks) {
    throw UnimplementedError(
        'setHandlerCallbacks() is not supported on this platform');
  }
}
