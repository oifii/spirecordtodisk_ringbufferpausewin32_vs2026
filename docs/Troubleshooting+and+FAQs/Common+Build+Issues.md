# Troubleshooting and FAQs – Common Build Issues

This section addresses frequent build errors encountered when compiling **Spirecordtodisk** with Visual Studio 2026. For each issue, you’ll find the typical **symptom**, a concise **resolution**, and examples of the relevant project settings.

| Issue | Symptom | Resolution |
| --- | --- | --- |
| 🛠️ Missing PortAudio or libsndfile headers | Compiler errors: `fatal error: portaudio.h: No such file` | Verify that **AdditionalIncludeDirectories** point to the PortAudio and libsndfile include folders. Adjust the paths under `lib-src` as needed. |
| 🔗 Linker cannot find `.lib` files | LNK1104: unable to open file `portaudio_x86.lib` or `libsndfile` | Ensure **AdditionalDependencies** reference the correct `.lib` files for your configuration/platform, and that those libraries have been built. |
| 🎛️ ASIO driver issues | Failure to open ASIO device or invalid `hostApiSpecificStreamInfo` | Confirm your ASIO driver is installed, and the device name matches PortAudio’s report. Fallback to WDM-KS or DirectSound by omitting ASIO name. |
| 🎙️ No default input device | Error: “Error: No default input device.” | Install and enable at least one recording device in Windows. Or explicitly pass a device name via the third argument. |


---

### 🛠️ Missing PortAudio or libsndfile headers

If the compiler cannot locate required headers, builds will fail immediately.

- **Symptom**
- `error C1083: Cannot open include file: 'portaudio.h': No such file or directory`
- Similar errors for `pa_ringbuffer.h` or `sndfile.hh`.
- **Resolution**
- Open **Project → Properties → C/C++ → Additional Include Directories**.
- Confirm the paths match your folder structure. By default, the project expects:

```xml
    <AdditionalIncludeDirectories>
      .\lib-src\portaudio-2021\portaudio_vs2026\include;
      .\lib-src\portaudio-2021\portaudio_vs2026\src\common;
      .\lib-src\libsndfile\include
    </AdditionalIncludeDirectories>
```

- Adjust these to the actual locations in your checkout.

---

### 🔗 Linker cannot find portaudio_x86.lib / portaudio_x64.lib or libsndfile-1.lib

When the linker cannot locate library files, you’ll see **LNK1104** errors.

- **Symptom**
- `LNK1104: cannot open file 'portaudio_x86.lib'`
- `LNK1104: cannot open file 'libsndfile-1.lib'`
- **Resolution**
- In **Project → Properties → Linker → Input → Additional Dependencies**, verify entries match the built library locations. Example for **Release | x64**:

```xml
    <AdditionalDependencies>
      .\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Release\portaudio_x64.lib;
      .\lib-src\libsndfile(x64)\lib\libsndfile-1.lib;
      %(AdditionalDependencies)
    </AdditionalDependencies>
```

- Make sure you have built PortAudio for both x86 and x64 (or have prebuilt `.lib` files) and that libsndfile’s `.lib` is present under the expected path.

---

### 🎛️ ASIO driver issues

PortAudio’s ASIO support can be sensitive to driver installation and naming.

- **Symptom**
- Stream fails to open when specifying an ASIO device.
- Errors related to `PaAsioStreamInfo` or invalid `hostApiSpecificStreamInfo`.
- **Resolution**
- Verify your ASIO driver is properly installed via Device Manager or vendor utility.
- Use `spidevicesselect.exe` (bundled under `lib-src\portaudio-2021\portaudio_vs2026\build\msvc`) to list exact device names.
- Ensure the name passed on the command line matches PortAudio’s output exactly (case and spacing).
- To debug, omit the ASIO device name argument—PortAudio will fall back to WDM-KS or DirectSound.

---

### 🎙️ No default input device

If PortAudio cannot find any enabled recording device, initialization will fail.

- **Symptom**
- `Error: No default input device.` printed at startup.
- **Resolution**
- Open **Settings → System → Sound**, and confirm at least one input device is enabled.
- If you want to bypass the default device, pass a specific device name as the third argument:

```bash
    spirecordtodisk_ringbufferpausewin32.exe test.wav 10 "Your Device Name"
```

- Use the device-selection utility (`spidevicesselect.exe`) to discover valid names.

---

**Still stuck?**

- Double-check that all header and library paths in **Property Manager** match your local layout.
- Clean and rebuild the solution after any path changes.
- Consult the PortAudio and libsndfile documentation for platform-specific quirks.

Happy recording!