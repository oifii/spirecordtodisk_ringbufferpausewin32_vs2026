# Getting Started – Quick Start: Record Your First WAV File

This guide walks you through recording a stereo WAV file with the `spirecordtodisk_ringbufferpausewin32.exe` tool. You’ll learn how to run the recorder, use default settings, customize filename and duration, select a specific audio device, pause/resume on the fly, and stop gracefully.

## 1. Launch the Recorder

After building the project in Visual Studio, locate the executable in your build output (e.g., `Debug\Win32` or `Release\x64`). Run it with no arguments:

```bash
spirecordtodisk_ringbufferpausewin32.exe
```

This starts recording immediately using the **default settings**.

## 2. Default Recording Settings

When run without any parameters, the application uses built-in defaults:

| Parameter | Default Value |
| --- | --- |
| **Output Filename** | `testrecording.wav` |
| **Duration** | `NUM_SECONDS` (60 seconds) |
| **Input Device** | First available ASIO or host‐API device |
| **Channels** | Left = 0, Right = 1 |


## 3. Specify Output Filename and Duration

> ℹ️ **Note**: `NUM_SECONDS` is defined in `spirecordtodisk_ringbufferpausewin32.h` as 60 seconds by default.

To record to a custom file for a defined time, pass the filename and seconds:

```bash
spirecordtodisk_ringbufferpausewin32.exe myrecording.wav 30
```

- `**myrecording.wav**` – Output file name
- `**30**` – Approximate recording length in seconds

The recorder stops after ~30 seconds (unless paused).

## 4. Choose a Specific Input Device and Channels

Append the ASIO device name and channel indices:

```bash
spirecordtodisk_ringbufferpausewin32.exe myrecording.wav 30 "E-MU ASIO" 0 1
```

| Argument Position | Description |
| --- | --- |
| 3 | ASIO device name (exact match) |
| 4 | Left channel index (e.g., `0`) |
| 5 | Right channel index (e.g., `1`) |


> ⚠️ The device string must exactly match the name reported by `spidevicesselect.exe`.

For a full list of command-line options and modes, see the **Command-Line Arguments and Modes** section of this documentation.

## 5. Pause and Resume Recording

While recording, press **P** to toggle pause/resume:

- ⏸️ **Pause** – Press **P** once
- ▶️ **Resume** – Press **P** again

The recorder continues running, and paused time is excluded from the total  . If you’ve configured on-screen text overlay via **spitext**, you’ll see the elapsed timer update automatically.

## 6. Stop and Finalize Recording

To end your session and finalize the WAV file:

- **Windows GUI**: Close the application window
- **Console**: Press **Ctrl+C**

On termination, the app gracefully stops the PortAudio stream, joins the file-writing thread, and writes the WAV header so your file is ready to play.

```c
int Terminate(FILE* pFILE) {
    err = Pa_CloseStream(stream);
    stopThread(&global_paTestData);
    return err;
}
```

---

```card
{
    "title": "Tip",
    "content": "Use a negative duration (e.g., `-1`) to record until you manually stop."
}
```