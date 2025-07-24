# Tag along

A sheet music viewer with special integration for [Barbershop tags](https://www.barbershoptags.com/).

# Installation

In the future this application will be available on the Google Play Store.

## Building from source

Make sure you have a C++26 compiler and the latest Qt with the Declarative, Svg, Pdf, Network and Sql modules installed.

```bash
cmake -B build
cmake --build build
```

For the Android build follow the [Qt for Android setup](https://doc.qt.io/qt-6/android.html), then build with:

```bash
~/Qt/*/android_arm64_v8a/bin/qt-cmake -DANDROID_SDK_ROOT=$HOME/Android/Sdk -DANDROID_NDK_ROOT=$HOME/Android/Sdk/ndk/* -B build -G Ninja
cmake --build build --target icon-integration
cmake --build build
```

You can then flash the APK with `adb install /path/to/android-build-debug.apk`.
