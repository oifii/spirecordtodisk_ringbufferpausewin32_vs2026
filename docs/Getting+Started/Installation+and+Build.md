# Getting Started - Installation and Build

This guide walks you through setting up and building **spirecordtodisk_ringbufferpausewin32** on Windows using Visual Studio. By the end, you'll have a working stereo-audio recorder with pause/resume and on-screen overlay.

## Prerequisites

- **Operating System**

Windows 10 or later (x86/x64).

- **IDE & Toolset**

Visual Studio 2022 (v145 toolset).

C++ Desktop Development workload.

- **Bundled Libraries**
- PortAudio (with ASIO support)
- libsndfile

## 1. Clone or Extract the Repository 🗂️

Ensure the **root folder** contains:

- `spirecordtodisk_ringbufferpausewin32.vcxproj`
- `lib-src` directory

```bash
git clone https://github.com/oifii/spirecordtodisk_ringbufferpausewin32_vs2026.git
cd spirecordtodisk_ringbufferpausewin32_vs2026
```

## 2. Open the Project in Visual Studio 💻

> If you download a ZIP, extract it so that the folder structure matches above.

1. Launch **Visual Studio 2022**.
2. Select **File → Open → Project/Solution…**
3. Navigate to the root folder and open `spirecordtodisk_ringbufferpausewin32.vcxproj`.

## 3. Configure Include Directories ⚙️

The project bundles PortAudio and libsndfile headers under `lib-src`.

Verify **AdditionalIncludeDirectories** in **Project → Properties → C/C++ → General**:

| Component | Path |
| --- | --- |
| PortAudio headers | `.\lib-src\portaudio-2021\portaudio_vs2026\include` |
| PortAudio common sources | `.\lib-src\portaudio-2021\portaudio_vs2026\src\common` |
| libsndfile headers | `.\lib-src\libsndfile\include` |


```xml
<AdditionalIncludeDirectories>
  .\lib-src\portaudio-2021\portaudio_vs2026\include;
  .\lib-src\portaudio-2021\portaudio_vs2026\src\common;
  .\lib-src\libsndfile\include;
  %(AdditionalIncludeDirectories)
</AdditionalIncludeDirectories>
```

## 4. Configure Linker Dependencies 🔗

Under **Project → Properties → Linker → Input**, confirm **AdditionalDependencies** for each configuration:

| Library | Path |
| --- | --- |
| `portaudio_x64.lib` | `.\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Release\portaudio_x64.lib` |
| `libsndfile-1.lib` | `.\lib-src\libsndfile(x64)\lib\libsndfile-1.lib` |


```xml
<AdditionalDependencies>
  .\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Release\portaudio_x64.lib;
  .\lib-src\libsndfile(x64)\lib\libsndfile-1.lib;
  %(AdditionalDependencies)
</AdditionalDependencies>
```

## 5. Select Configuration & Platform 🔄

> For Win32 builds, adjust paths to the x86 subdirectories accordingly.

In the Visual Studio toolbar:

1. **Configuration**: choose **Debug** or **Release**
2. **Platform**: choose **Win32** or **x64**

These settings control optimization, debug symbols, and the correct libraries.

## 6. Build the Solution 🚀

Press **F7** or select **Build → Build Solution**. This compiles:

- **Main Application** (`spirecordtodisk_ringbufferpausewin32.cpp`)
- **SPIAudioDevice Helper** (`spiaudiodevice.cpp`)
- **Text Overlay Module** (`spitext.cpp`)
- **Utilities** (`spiutility.cpp`)
- **Precompiled Headers** (`stdafx.cpp/.h`)
- **PortAudio Support Sources**:
- `pa_ringbuffer.c`
- `pa_win_util.c`

Upon success, the executable appears in `.\$(Configuration)\$(Platform)\`.

---

Congratulations! You’ve set up and built **spirecordtodisk_ringbufferpausewin32**. Next, explore running the application and using keyboard controls for pause/resume and on-screen timers.