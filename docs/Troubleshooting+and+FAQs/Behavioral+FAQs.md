# Troubleshooting and FAQs – Behavioral FAQs

This section answers common behavioral questions about spirecordtodisk – how recording, timing, device selection, and overlay features work at runtime. Each Q&A explains observed behaviors and configuration options.

## 1. Does recording continue while the app is paused? ❓

When you press **P**, recording “pauses,” but PortAudio’s callback still runs.

- **Audio callback** always executes.
- **Disk writes** are suspended while `global_pauserecording` is `true`.
- On resume, writing restarts automatically.
- To avoid audible clicks, the first resumed buffer is muted when `global_prev_pauserecording` was `true`:

| State | Audio Callback | Disk Write | First Resume Output |
| --- | --- | --- | --- |
| Paused | Running | Suspended | Muted (zeroed buffer) |
| Recording | Running | Active (ring→disk) | Live (normal playback) |


```cpp
// In recordCallback: skip writing when paused
if (global_pauserecording)
    return paContinue;
…
data->frameIndex += PaUtil_WriteRingBuffer(&data->ringBuffer, rptr, elementsToWrite);
```

```cpp
// Mute one buffer on resume to avoid clicks
if (global_prev_pauserecording) {
    global_prev_pauserecording = false;
    memset(out, 0, framesPerBuffer * NUM_CHANNELS * sizeof(SAMPLE));
} else {
    memcpy(out, in, framesPerBuffer * NUM_CHANNELS * sizeof(SAMPLE));
}
```

**Why?** This design ensures you can pause file writes without interrupting real-time playback and prevent pops on resume.

## 2. Can I record longer than **NUM_SECONDS**? ⏲️

By default, the app records for `NUM_SECONDS` (60 s). You can override this via the **second** command-line argument:

- **Positive duration**: records for that many seconds.
- **Negative duration**: disables time-based stop; recording continues until you terminate the process (Ctrl+C or console close).

```bash
# Record for 120 seconds instead of default 60
spirecord test.wav 120 "My ASIO Device" 0 1
```

```cpp
// In _tWinMain: parse duration argument
float fSecondsRecord = NUM_SECONDS;
if (nArgs > 2) {
    fSecondsRecord = atof(szArgList[2]); 
    // fSecondsRecord > 0 used only if no overlay start/end timers
}
…
while (delayCntr < fSecondsRecord) {
    Pa_Sleep(1000);
    if (!global_pauserecording) ++delayCntr;
}
```

**Tip:** For unlimited recording, pass a negative number.

## 3. How do I find the exact device names for arguments 3 and 6? 🔍

The app matches audio device names **exactly** as reported by PortAudio. To list available devices:

1. Use the helper `spidevicesselect.exe` (in `audio_spi\spidevicesselect.exe`) to enumerate device names.
2. Alternatively, extend `SPIAudioDevice::SelectAudioInputDevice` or output a PortAudio device list at startup.

```cpp
// Usage comment in code
// use audio_spi\spidevicesselect.exe to find the exact device name
// (must match string detected by PortAudio)
```

```bash
# Example: list devices and pick the exact name
audio_spi\spidevicesselect.exe
```

**Note:** This utility isn’t part of the main solution but demonstrates how to retrieve correct strings.

## 4. Can I use the overlay without recording audio? 🎨

Yes. The **spitext** module is a standalone Win32 overlay engine. You can compile and run it separately to display timers or custom text on screen:

- **Independent entry point**: its own `WinMain`, resources, and message loop.
- **Configuration** via global variables (e.g., `global_spitextstring`, `global_starttime_sec`, `global_endtime_sec`).
- **Positioning**: set `global_x`, `global_y`, font size, color, justification.

```bash
# Example: launch overlay only
spitext.exe "COUNTUP MM:SS" -1 -1 100 200 24 "Arial" 0
```

```cpp
// In spirecordtodisk module, overlay args parsing
if (nArgs > (1 + nargs_recordtodisk))
    global_spitextstring = szArgList[1 + nargs_recordtodisk];
if (nArgs > (2 + nargs_recordtodisk))
    global_starttime_sec = atoi(szArgList[2 + nargs_recordtodisk]);
…
```

Because **spitext** doesn’t depend on PortAudio or ring buffers, you can integrate it into other C++ projects for custom on-screen text.

---

If these FAQs don’t resolve your issue, consult error output or enable verbose logging by modifying the code to print device lists and state changes. Happy recording!