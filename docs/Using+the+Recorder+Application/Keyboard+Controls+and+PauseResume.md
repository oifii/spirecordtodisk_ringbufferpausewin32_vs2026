# Using the Recorder Application – Keyboard Controls and Pause/Resume

This section explains how to control recording via the keyboard and how pause/resume is implemented under the hood. You’ll learn how key events toggle global flags, how the PortAudio callback behaves when paused, and how on-screen timing adjusts to ignore pause durations.

## ⚙️ Keyboard Input Monitoring

The application watches for the **P** key to toggle recording. In console mode it uses functions from `<conio.h>`, while the GUI overlay handles key events via `WM_KEYDOWN`.

- **Console Mode**
- Calls `_kbhit()` to check for any key press.
- Reads the key with `_getch()`.
- Toggles pause when the character is `'p'` or `'P'`.
- **Overlay Window Mode**
- Registers a Windows message handler in `WndProc`.
- Catches `WM_KEYDOWN` for `wParam == 0x50` (the P key).

```cpp
// Console loop: check for 'P', toggle global_pauserecording
while( delayCntr < fSecondsRecord ) {
    if (_kbhit() && _getch()=='p') {
        if (!global_pauserecording) {
            global_pauserecording = true;
            printf("pause pressed\n");
        } else {
            global_pauserecording = false;
            printf("unpause pressed\n");
        }
        fflush(stdout);
    }
    Pa_Sleep(1000);
    if (!global_pauserecording) delayCntr++;
}
```

—

## 🛑 Pause State Management

Several **global flags** coordinate pause behavior throughout the app:

| Flag | Type | Purpose |
| --- | --- | --- |
| **global_pauserecording** | bool | `true` when recording is paused; suppresses writes to disk. |
| **global_prev_pauserecording** | bool | Tracks previous pause state; used to zero audio output once on resuming. |
| **global_pausestartstamp_ms** | DWORD | Timestamp when pause began (`GetTickCount()`). |
| **global_pausetimetotal_ms** | DWORD | Accumulates total milliseconds spent paused, for timer adjustment. |
| **global_pausedisabled** | bool | When `true`, disables pause toggling and hides the overlay’s taskbar icon. |


## 🎧 Pause/Resume in PortAudio Callback

The `recordCallback` always runs but **suppresses writing** to the ring buffer when paused. Playback of the live input continues; on resume, the first buffer is zeroed to prevent an audible click.

```cpp
// Always output live audio, even when paused
const SAMPLE* in  = (const SAMPLE*) inputBuffer;
SAMPLE*       out = (SAMPLE*)       outputBuffer;
if (out) {
    if (global_prev_pauserecording) {
        global_prev_pauserecording = false;
        memset(out, 0, framesPerBuffer * NUM_CHANNELS * sizeof(SAMPLE));
    } else {
        memcpy(out, in, framesPerBuffer * NUM_CHANNELS * sizeof(SAMPLE));
    }
}
// If paused, skip writing to disk
if (global_pauserecording) return paContinue;

// Normal recording: write into ring buffer
paTestData *data = (paTestData*) userData;
ring_buffer_size_t writeAvail = PaUtil_GetRingBufferWriteAvailable(&data->ringBuffer);
ring_buffer_size_t toWrite    = min(writeAvail, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
data->frameIndex += PaUtil_WriteRingBuffer(&data->ringBuffer, in, toWrite);
return paContinue;
```

—

## ⏲️ Timer Adjustment for Pause

The on-screen timer subtracts pause durations so the display reflects **active recording time**. When paused, `WM_PAINT` logic computes elapsed seconds, deducting any paused intervals:

```cpp
// Compute elapsed seconds excluding pause time
int elapsed_sec = (global_nowstamp_ms - global_startstamp_ms) / 1000;
if (global_dontcounttimeonpause) {
    DWORD currentPause_ms = global_pausestartstamp_ms
        ? (global_nowstamp_ms - global_pausestartstamp_ms)
        : 0;
    int totalPause_sec = (global_pausetimetotal_ms + currentPause_ms) / 1000;
    elapsed_sec -= totalPause_sec;
}
global_timetodisplay_sec = global_starttime_sec + elapsed_sec;
InvalidateRect(hWnd, NULL, FALSE);
```

—

## 🚫 Passive Overlay Mode (Disable Pause)

To run the overlay **without** pause controls or a taskbar icon, set **DISABLEPAUSE** in the mode string. During window creation, this flag alters extended styles:

```cpp
// In InitInstance
DWORD exFlags = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
if (!global_pausedisabled) {
    // Remove NOACTIVATE to allow key events and show taskbar icon
    exFlags = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
}
HWND hWnd = CreateWindowEx(exFlags, szWindowClass, szTitle, WS_POPUP,
                          0, 0, 100, 100, NULL, NULL, hInstance, NULL);
```

—

## Summary

- Press **P** to **pause** or **resume** recording.
- The **PortAudio callback** continues but skips disk writes when paused.
- **Playback** remains live, with a brief silence buffer on resume to eliminate glitches.
- The **on-screen timer** adjusts for pause durations.
- Enable **DISABLEPAUSE** mode for a passive overlay with no keyboard controls.