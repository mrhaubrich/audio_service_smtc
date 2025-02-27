/// Callbacks that can be invoked by the platform implementation
abstract class AudioHandlerCallbacks {
  /// Request to play
  void play(PlayRequest request);

  /// Request to pause
  void pause(PauseRequest request);

  /// Request to stop
  void stop(StopRequest request);

  /// Request to seek
  void seek(SeekRequest request);

  /// Request to skip to next item
  void skipToNext(SkipToNextRequest request);

  /// Request to skip to previous item
  void skipToPrevious(SkipToPreviousRequest request);

  /// Request to play from a URI
  void playFromUri(PlayFromUriRequest request);

  /// Request to perform a custom action
  void customAction(CustomActionRequest request);
}

/// Play request
class PlayRequest {
  /// Creates a new play request
  const PlayRequest();
}

/// Pause request
class PauseRequest {
  /// Creates a new pause request
  const PauseRequest();
}

/// Stop request
class StopRequest {
  /// Creates a new stop request
  const StopRequest();
}

/// Seek request
class SeekRequest {
  /// Creates a new seek request
  const SeekRequest({required this.position});

  /// Position to seek to
  final Duration position;
}

/// Skip to next request
class SkipToNextRequest {
  /// Creates a new skip to next request
  const SkipToNextRequest();
}

/// Skip to previous request
class SkipToPreviousRequest {
  /// Creates a new skip to previous request
  const SkipToPreviousRequest();
}

/// Play from URI request
class PlayFromUriRequest {
  /// Creates a new play from URI request
  const PlayFromUriRequest({required this.uri});

  /// URI to play from
  final Uri uri;
}

/// Custom action request
class CustomActionRequest {
  /// Creates a new custom action request
  const CustomActionRequest({
    required this.name,
    this.extras,
  });

  /// Name of the action
  final String name;

  /// Extra data for the action
  final Map<String, dynamic>? extras;
}
