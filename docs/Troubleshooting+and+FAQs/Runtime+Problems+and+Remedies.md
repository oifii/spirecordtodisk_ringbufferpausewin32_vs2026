# Troubleshooting and FAQs – Runtime Problems and Remedies

This section helps diagnose common runtime issues in the recorder application and offers solutions. Follow the guidance below to restore reliable recording, monitoring, pause/resume behavior, and overlay display.

---

## 1. Recording Stops Prematurely or WAV File Is Incomplete 🎵

When your recording ends before the specified duration or the resulting WAV file is truncated:

| Possible Cause | Remedy |
| --- | --- |
| **Insufficient ring buffer size** | Increase the buffer duration factor. In the initialization code, adjust the `0.5`-second factor to a larger value (e.g., `1.0` for 1 second):  <br/>```cpp<br/>numSamples = NextPowerOf2((unsigned)(SAMPLE_RATE * 1.0 * NUM_CHANNELS));<br/>``` |
| **High disk I/O latency** | Record to a faster drive or SSD. Close other disk-heavy applications during recording. |
| **Abrupt termination (e.g., console window killed)** | Always exit via normal window close or Ctrl+C to trigger `Terminate()`, which stops the stream, flushes threads, and closes the file cleanly:  <br/>```cpp<br/>Terminate(NULL);<br/>SetEvent(g_hTerminateEvent);<br/>``` |


---

## 2. No Sound in Monitoring Output 🔇

If you hear nothing during recording despite setting an output device:

- **Verify device name argument**

Ensure you passed a valid output device name as the 6th CLI argument. In `WinMain`, this is assigned to `mySPIAudioDevice.global_audiooutputdevicename` before calling `SelectAudioOutputDevice()` .

- **Check **`**SelectAudioOutputDevice()**`** success**

Confirm `global_outputParameters` is populated. If you leave the output device string empty, monitoring is disabled—use this to isolate input-only issues.

- **Confirm host API support**

Only ASIO or WDMKS backends are tested. If uncertain, start with no output device to ensure input capture is functioning, then re-enable output.

---

## 3. Pause Key (P) Has No Effect ⏸️

Pressing **P** should toggle pause/resume recording. If it doesn’t:

- **Pause feature disabled at build/config time**

If `global_pausedisabled` is set to `true`, the pause handler is skipped. This flag is toggled when the modestring contains `"DISABLEPAUSE"`:

```cpp
  if (global_modestring.find("DISABLEPAUSE") != std::string::npos) {
      global_pausedisabled = true;
  }
```

Rebuild or run without the `DISABLEPAUSE` mode .

- **Window focus missing**

The overlay or console window must have keyboard focus for `_kbhit()`/`_getch()` to detect **P**. Click the window or taskbar icon before pressing **P**.

---

## 4. Overlay Window Not Visible or Mispositioned 🖥️

If on-screen timer/text doesn’t appear or is off-screen:

| Configuration Parameter | Troubleshooting Step |
| --- | --- |
| **global_hwnd** / **global_windowclass** / **global_windowtitle** | Verify your target window identifiers match exactly. Typos cause `GetWindowRect()` to fail, logging errors such as:<br/>```txt<br/>error global_windowtitle not found<br/>``` |
| **global_monitor** / **global_hmonitor** | Ensure correct monitor index or region syntax (`"1"`, `"1.1"`, `"1:1"`). Incorrect values may place text off-screen. |
| **Opacity & Keying** | Check alpha value and chroma-key color. If `global_alpha` is too low or `global_keyingcolor` (default `RGB(255,0,255)`) matches your background, text may be invisible. Adjust in code. |


---

## Quick Reference: Command-Line Arguments

```bash
spirecordtodisk_ringbufferpausewin32.exe \
  <output.wav> \
  <duration_sec> \
  "<ASIO or WDMKS device name>" \
  <input-channel-idx> \
  <input-channel-idx-2> \
  <output-device-name> \
  <output-channel-idx-1> \
  <output-channel-idx-2> \
  "<modestring>"
```

- **modestring** can include:
- `PAUSEONSTART`
- `DISABLEPAUSE`
- `DONTCOUNTTIMEONPAUSE`

Adjust these to enable/disable features during debugging.

---

### Additional Tips

- Always monitor the debug log (`stderr`) for error messages.
- Use small test recordings (e.g., 5 sec) to iterate quickly.
- Update to the latest PortAudio and libsndfile versions for stability.

Happy recording!