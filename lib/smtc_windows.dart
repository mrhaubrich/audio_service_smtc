// import 'dart:async';
// import 'dart:developer';
// import 'dart:ffi';

// import 'package:audio_service_smtc/src/metadata.dart';
// import 'package:ffi/ffi.dart';
// import 'package:win32/win32.dart';

// /// Interface for SystemMediaTransportControlsManager COM object
// class ISystemMediaTransportControlsManager extends IUnknown {
//   ISystemMediaTransportControlsManager(super.ptr);

//   Pointer<COMObject> getForCurrentView() {
//     final retval = calloc<COMObject>();
//     final hr = ptr.ref.vtable
//         .elementAt(3)
//         .cast<
//             Pointer<
//                 NativeFunction<
//                     HRESULT Function(
//                       LPVTBL lpVtbl,
//                       Pointer<COMObject> retval,
//                     )>>>()
//         .value
//         .asFunction<int Function(LPVTBL lpVtbl, Pointer<COMObject> retval)>()(
//       ptr.ref.lpVtbl,
//       retval,
//     );

//     if (FAILED(hr)) {
//       calloc.free(retval);
//       throw WindowsException(hr);
//     }

//     return retval;
//   }
// }

// /// GUID for SystemMediaTransportControlsManager class
// // ignore: non_constant_identifier_names
// final CLSID_SystemMediaTransportControlsManager = GUIDFromString(
//   '{2C8C6CC0-7E8D-44A6-8F2E-4C6E1EC9F5D3}',
// );

// /// GUID for ISystemMediaTransportControlsManager interface
// // ignore: non_constant_identifier_names
// final IID_ISystemMediaTransportControlsManager = GUIDFromString(
//   '{0C012146-9B92-44D2-9C13-2B2C6BBDA3D8}',
// );

// /// Windows implementation for System Media Transport Controls (SMTC)
// class SmtcWindows {
//   /// Constructor for SMTC Windows handler
//   SmtcWindows({required this.identity}) {
//     _initialize();
//   }

//   /// The identity of the media player
//   final String identity;

//   Pointer<COMObject>? _smtcManager;
//   Pointer<COMObject>? _mediaTransportControls;
//   Pointer<COMObject>? _displayUpdater;
//   Pointer<COMObject>? _musicProperties;

//   final _controlStreamController = StreamController<String>.broadcast();

//   /// Stream of control events from SMTC
//   Stream<String> get controlStream => _controlStreamController.stream;

//   final _positionStreamController = StreamController<Duration>.broadcast();

//   /// Stream of position change events
//   Stream<Duration> get positionStream => _positionStreamController.stream;

//   Duration _position = Duration.zero;

//   /// Current playback position
//   Duration get position => _position;

//   /// Set playback position
//   set position(Duration value) {
//     _position = value;
//     // Implementation for position update in SMTC
//   }

//   String _playbackState = 'Stopped';

//   /// Current playback state
//   String get playbackState => _playbackState;

//   /// Set playback state
//   set playbackState(String state) {
//     if (state == _playbackState) return;

//     _updatePlaybackStatus(state);
//     _playbackState = state;
//   }

//   /// Initialize SMTC
//   void _initialize() {
//     try {
//       final hr = CoInitializeEx(
//         nullptr,
//         COINIT.COINIT_APARTMENTTHREADED | COINIT.COINIT_DISABLE_OLE1DDE,
//       );

//       if (FAILED(hr)) {
//         log(
//           'Failed to initialize COM: ${hr.toHexString(32)}',
//           name: 'audio_service_smtc',
//         );
//         return;
//       }

//       _smtcManager = createSystemMediaTransportControlsManager();
//       _registerForEvents();
//     } catch (e) {
//       log('Error initializing SMTC: $e', name: 'audio_service_smtc');
//     }
//   }

//   /// Create SMTC manager
//   Pointer<COMObject> createSystemMediaTransportControlsManager() {
//     final manager = calloc<COMObject>();
//     final hr = CoCreateInstance(
//       CLSID_SystemMediaTransportControlsManager.ref as Pointer<GUID>,
//       nullptr,
//       CLSCTX.CLSCTX_INPROC_SERVER,
//       IID_ISystemMediaTransportControlsManager.ref,
//       manager.cast(),
//     );

//     if (FAILED(hr)) {
//       calloc.free(manager);
//       throw WindowsException(hr);
//     }

//     final smtcManager = ISystemMediaTransportControlsManager(manager);
//     _mediaTransportControls = smtcManager.getForCurrentView();

//     final controls = ISystemMediaTransportControls(_mediaTransportControls!);
//     controls.isEnabled = true;
//     controls.isPlayEnabled = true;
//     controls.isPauseEnabled = true;
//     controls.isNextEnabled = true;
//     controls.isPreviousEnabled = true;
//     controls.isStopEnabled = true;

//     _displayUpdater = controls.displayUpdater;

//     return manager;
//   }

//   /// Register for SMTC events
//   void _registerForEvents() {
//     if (_mediaTransportControls == null) return;

//     final controls = ISystemMediaTransportControls(_mediaTransportControls!);

//     // Register for button pressed events
//     controls.onButtonPressed.toFunction((sender, args) {
//       final button = args.ref.button;
//       switch (button) {
//         case SystemMediaTransportControlsButton.play:
//           _controlStreamController.add('play');
//         case SystemMediaTransportControlsButton.pause:
//           _controlStreamController.add('pause');
//         case SystemMediaTransportControlsButton.next:
//           _controlStreamController.add('next');
//         case SystemMediaTransportControlsButton.previous:
//           _controlStreamController.add('previous');
//         case SystemMediaTransportControlsButton.stop:
//           _controlStreamController.add('stop');
//         default:
//           // Ignore other buttons
//           break;
//       }
//       return S_OK;
//     });
//   }

//   /// Update playback status in SMTC
//   void _updatePlaybackStatus(String state) {
//     if (_mediaTransportControls == null) return;

//     final controls = ISystemMediaTransportControls(_mediaTransportControls!);

//     switch (state) {
//       case 'Playing':
//         controls.playbackStatus = MediaPlaybackStatus.playing;
//       case 'Paused':
//         controls.playbackStatus = MediaPlaybackStatus.paused;
//       case 'Stopped':
//         controls.playbackStatus = MediaPlaybackStatus.stopped;
//       default:
//         controls.playbackStatus = MediaPlaybackStatus.closed;
//     }
//   }

//   /// Update metadata in SMTC
//   void updateMetadata(SmtcMetadata metadata) {
//     if (_displayUpdater == null) return;

//     try {
//       final updater =
//           ISystemMediaTransportControlsDisplayUpdater(_displayUpdater!);

//       // Clear any existing metadata
//       updater.type = MediaPlaybackType.music;

//       // Get the music properties
//       _musicProperties = updater.musicProperties;
//       final properties = IMusicDisplayProperties(_musicProperties!);

//       // Update properties
//       properties.title = metadata.title;
//       if (metadata.artist != null) {
//         properties.artist = metadata.artist!.join(', ');
//       }
//       if (metadata.album != null) {
//         properties.albumTitle = metadata.album;
//       }

//       // Apply changes
//       updater.update();
//     } catch (e) {
//       log('Error updating metadata: $e', name: 'audio_service_smtc');
//     }
//   }

//   /// Sends seek position event
//   void seekTo(Duration position) {
//     _positionStreamController.add(position);
//   }

//   /// Clean up resources
//   void dispose() {
//     _controlStreamController.close();
//     _positionStreamController.close();

//     if (_musicProperties != null) {
//       _musicProperties!.release();
//       _musicProperties = null;
//     }

//     if (_displayUpdater != null) {
//       _displayUpdater!.release();
//       _displayUpdater = null;
//     }

//     if (_mediaTransportControls != null) {
//       _mediaTransportControls!.release();
//       _mediaTransportControls = null;
//     }

//     if (_smtcManager != null) {
//       _smtcManager!.release();
//       _smtcManager = null;
//     }

//     CoUninitialize();
//   }
// }
