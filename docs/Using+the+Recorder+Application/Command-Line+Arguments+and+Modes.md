# Using the Recorder Application – Command-Line Arguments and Modes

The recorder parses its invocation via `CommandLineToArgvA/W` in the Windows entry point `_tWinMain`, exposing a simple, positional argument scheme. You can override defaults to customize output file, duration, device routing, monitoring, and initial pause behavior .

## Command-Line Arguments

Below is a summary of the first six command-line parameters:

🎯 **Argument Reference**

| Argument | Description | Default | Example |
| --- | --- | --- | --- |
| 1 | **Output filename** | `testrecording.wav` | `session1.wav` |
| 2 | **Recording duration** (seconds; float) | Value of `NUM_SECONDS`¹ | `15.5` |
| 3 | **Audio input device name** (exact, as shown by `spidevicesselect.exe`) | ; must supply | `"E-MU ASIO"` |
| 4 | **ASIO input channel** for left channel (zero-based index) | `0` | `2` (third ASIO channel) |
| 5 | **ASIO input channel** for right channel (zero-based index) | `1` | `3` (fourth ASIO channel) |
| 6 | **Audio output device name** (for live monitoring) | `""` (disabled) | `"E-MU ASIO"` |


¹ `NUM_SECONDS` is defined in code (default 60 sec) .

### Detailed Argument Behavior

- **Argument 1**: Sets the WAV file to write. If omitted, the app writes to `testrecording.wav`.
- **Argument 2**: Governs how long the app records (in seconds).
- If positive **and** no explicit start/end timers are set (`global_starttime_sec` and `global_endtime_sec` both negative), the app stops after this duration.
- If negative (and no timers), recording runs until manual termination.
- **Argument 3**: Matches exactly one of the names returned by the helper tool `spidevicesselect.exe`.
- **Arguments 4 & 5**: Choose which ASIO channels feed left/right. Useful on multi-input interfaces.
- **Argument 6**: When non-empty, configures an output stream for live audio monitoring while recording.

```bash
# Basic usage: default filename, 10 sec, default devices/channels
spirecordtodisk_ringbufferpausewin32.exe

# Specify filename and duration only
spirecordtodisk_ringbufferpausewin32.exe session1.wav 12.75

# Full example with ASIO device & channels
spirecordtodisk_ringbufferpausewin32.exe session2.wav 20 "E-MU ASIO" 2 3
```

## Global Modes

A seventh, **optional** parameter (`global_modestring`) modifies startup pause behavior. Recognized flags (case-sensitive substrings) include:

- **DISABLEPAUSE**: Completely disables the pause/unpause feature.
- **PAUSEONSTART**: Begins recording in a paused state.
- **DONTCOUNTTIMEONPAUSE**: Excludes paused intervals from the displayed/recorded timer.

If multiple flags appear in the string, all matching effects apply .

### Mode Examples

```bash
# Start paused, then unpause via 'P' key
spirecordtodisk_ringbufferpausewin32.exe out.wav 30 "E-MU ASIO" 0 1 "" PAUSEONSTART

# Disable pause entirely
spirecordtodisk_ringbufferpausewin32.exe out.wav 30 "E-MU ASIO" 0 1 "" DISABLEPAUSE

# Pause on start and don't count paused time
spirecordtodisk_ringbufferpausewin32.exe out.wav 30 "E-MU ASIO" 0 1 "" "PAUSEONSTART;DONTCOUNTTIMEONPAUSE"
```

> **Note:** Mode flags are parsed in the main recorder logic and set global flags like `global_pauserecording`, `global_pausedisabled`, and `global_dontcounttimeonpause` before audio processing begins .

---

This section empowers you to tailor recording sessions from the command line—customizing file names, durations, device routing, live monitoring, and initial pause states with ease.