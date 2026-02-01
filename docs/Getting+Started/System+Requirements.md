## Getting Started – System Requirements

Prepare your development environment to build and run **spirecordtodisk_ringbufferpausewin32** on Windows.

### 🖥️ Supported Platforms

Supported target platforms for this application:

- **Windows desktop** (Win32 or x64)
- Requires **Windows 10 SDK**, targeting `WindowsTargetPlatformVersion = 10.0`
- Builds with both Debug and Release configurations

### 🛠️ Development Tools

Set up Microsoft Visual Studio with the following components:

- **Visual Studio 2019** (or later) with the **C++ Desktop development** workload
- **Platform Toolset**
- Configured to **v142** or newer (this project uses `v145`)
- No precompiled headers are used; the project disables `/Yu` and sets `<PrecompiledHeader>NotUsing</PrecompiledHeader>`

```xml
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
  <PlatformToolset>v145</PlatformToolset>
  <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
</PropertyGroup>
```

### 📦 Libraries & Dependencies

This application bundles and references two core audio libraries under the **lib-src** folder:

| Library | Include Path | Link Library |
| --- | --- | --- |
| **PortAudio** | `.\lib-src\portaudio-2021\portaudio_vs2026\include` <br/> `.\lib-src\portaudio-2021\portaudio_vs2026\src\common` | `.\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\<Platform>\<Configuration>\portaudio_x86(x64).lib` |
| **libsndfile** | `.\lib-src\libsndfile\include` | `.\lib-src\libsndfile\libsndfile-1.lib` |


- **ASIO support** is enabled via the preprocessor definition:

```xml
  <PreprocessorDefinitions>…;PA_USE_ASIO=1;…</PreprocessorDefinitions>
```

- Ring-buffer utilities (`pa_ringbuffer.c`) and Windows helpers (`pa_win_util.c`) are compiled directly into the project

### 🎚️ Audio Hardware Requirements

To record stereo audio reliably:

- **Input device** with **at least two channels** (stereo)
- **ASIO drivers** (optional) – if your sound card provides an ASIO interface, you’ll benefit from lower latency
- By default, ASIO is selected when `PA_USE_ASIO=1` and the host API type is `paASIO`

### ⚙️ Optional Components

- **Monitoring output**: an output device (speakers or headphones) allows real-time playback during recording
- **On-screen overlay**: requires a display environment (Windows GUI) for the built-in timer/text overlay
- **spitext.exe** (if present) can be used to render text on a secondary monitor or specified monitor quadrant

---

Follow these requirements to ensure a smooth build and runtime experience. Enjoy recording with controlled pause/resume and robust ring-buffered I/O!