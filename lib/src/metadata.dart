/// Metadata class for System Media Transport Controls
class SmtcMetadata {
  /// Creates a new instance of SmtcMetadata
  SmtcMetadata({
    required this.title,
    this.artist,
    this.album,
    this.albumArtist,
    this.trackNumber,
    this.duration,
    this.albumArtUrl,
    this.genre,
  });

  /// The title of the media
  final String title;

  /// The artist of the media
  final List<String>? artist;

  /// The album of the media
  final String? album;

  /// The album artist of the media
  final List<String>? albumArtist;

  /// The track number
  final int? trackNumber;

  /// The duration of the media
  final Duration? duration;

  /// URL to the album art
  final String? albumArtUrl;

  /// Genre of the media
  final List<String>? genre;

  /// Creates a copy of this metadata with the given fields replaced
  SmtcMetadata copyWith({
    String? title,
    List<String>? artist,
    String? album,
    List<String>? albumArtist,
    int? trackNumber,
    Duration? duration,
    String? albumArtUrl,
    List<String>? genre,
  }) {
    return SmtcMetadata(
      title: title ?? this.title,
      artist: artist ?? this.artist,
      album: album ?? this.album,
      albumArtist: albumArtist ?? this.albumArtist,
      trackNumber: trackNumber ?? this.trackNumber,
      duration: duration ?? this.duration,
      albumArtUrl: albumArtUrl ?? this.albumArtUrl,
      genre: genre ?? this.genre,
    );
  }
}
