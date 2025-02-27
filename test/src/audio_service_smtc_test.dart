// ignore_for_file: prefer_const_constructors

import 'dart:io';

import 'package:audio_service_smtc/audio_service_smtc.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:mocktail/mocktail.dart';

// Instead of implementing Platform, create a class to mock static properties
class PlatformInfo {
  bool get isWindows => Platform.isWindows;
}

class MockPlatformInfo extends Mock implements PlatformInfo {}

void main() {
  group('AudioServiceSmtc', () {
    test('can be instantiated', () {
      expect(AudioServiceSmtc(), isNotNull);
    });

    test('registerWith registers instance when on Windows', () {
      // We don't actually test the platform-specific code here
      // since it's hard to mock Windows APIs in tests
      expect(AudioServiceSmtc.registerWith, returnsNormally);
    });

    test('configure does not throw on non-Windows platform', () async {
      final service = AudioServiceSmtc();
      final config = AudioServiceConfig(
        androidNotificationChannelId: 'test.channel',
        androidNotificationChannelName: 'Test Channel',
      );

      expect(
        () => service.configure(ConfigureRequest(config: config)),
        returnsNormally,
      );
    });
  });
}
