# Configuration and Tuning - Audio Format and Ring Buffer Parameters

This section explains the key constants and calculations that control the audio format, recording duration, and ring-buffer sizing in the **spirecordtodisk_ringbufferpausewin32** application. Adjusting these values lets you balance latency, memory usage, and robustness against disk I/O delays.

## 🎚️ Core Recording Parameters

The following macros in `spirecordtodisk_ringbufferpausewin32.h` define the defaults for file naming, duration, buffer writes, and dithering:

| Parameter | Default Value | Purpose |
| --- | --- | --- |
| **FILE_NAME** | `"audio_data.raw"` | Raw-file name used internally by write threads. |
| **NUM_SECONDS** | `60` | Default recording length in seconds when no CLI duration is specified. |
| **NUM_WRITES_PER_BUFFER** | `4` | Number of ring-buffer segments to write per background thread cycle. |
| **DITHER_FLAG** | `0` | Dithering mode passed to PortAudio (currently disabled). |


```c
#define FILE_NAME              "audio_data.raw"
#define NUM_SECONDS            (60)
#define NUM_WRITES_PER_BUFFER  (4)
//#define DITHER_FLAG          (paDitherOff)
#define DITHER_FLAG           (0)
```

The `NUM_WRITES_PER_BUFFER` parameter determines how often the file‐writing thread wakes up. A higher value reduces latency but increases CPU wakeups  .

## 🎵 Audio Format Settings

Audio format parameters live in `defs.h` and must match between PortAudio and libsndfile:

```c
#define PA_SAMPLE_TYPE   paFloat32
typedef float SAMPLE;
#define SAMPLE_RATE      (44100)
#define FRAMES_PER_BUFFER (2048)
#define NUM_CHANNELS     (2)
```

- **PA_SAMPLE_TYPE** / **SAMPLE**: Defines the sample data type (float, int16, etc.).
- **SAMPLE_RATE**: Samples per second.
- **NUM_CHANNELS**: Number of audio channels (stereo = 2).
- **FRAMES_PER_BUFFER**: PortAudio callback frame size.

Change these to support mono, different sample rates, or integer formats. Ensure libsndfile uses the same `NUM_CHANNELS` and `SAMPLE_RATE` when writing WAV .

## 💾 Ring Buffer Sizing & Initialization

To buffer incoming audio before disk writes, the code allocates a ring buffer sized to approximately **0.5 seconds** of audio:

```c
unsigned int numSamples = NextPowerOf2((unsigned)(SAMPLE_RATE * 0.5 * NUM_CHANNELS));
unsigned int numBytes   = numSamples * sizeof(SAMPLE);

data.ringBufferData = (SAMPLE*) PaUtil_AllocateMemory(numBytes);
if (PaUtil_InitializeRingBuffer(
        &data.ringBuffer,
        sizeof(SAMPLE),
        numSamples,
        data.ringBufferData) < 0)
{
    // Error: buffer size must be power of two
}
```

- **NextPowerOf2(...)** rounds up to the nearest power of two for efficient indexing.
- **PaUtil_AllocateMemory** reserves contiguous memory for the buffer.
- **PaUtil_InitializeRingBuffer** sets up pointers and indices.

Allocating for 0.5 s helps absorb short disk‐write stalls. See the snippet in the initialization block .

## ⚙️ Practical Tuning Tips

- **Adjust Buffer Duration**

To improve robustness against disk latency, increase the **0.5** multiplier in the sizing formula (e.g., `0.75` for 750 ms). This raises memory usage and lengthens shutdown flush time.

- **Fine-Tune NUM_WRITES_PER_BUFFER**
- Lower values: Larger write chunks, lower CPU overhead, higher latency.
- Higher values: Smaller chunks, more frequent wakeups, lower latency.
- **Modify Recording Duration at Runtime**

Override **NUM_SECONDS** via the second CLI argument (`argv[2]`) to avoid recompilation.

- **Match Formats**

Always ensure `SAMPLE_RATE`, `NUM_CHANNELS`, and `PA_SAMPLE_TYPE` are identical in PortAudio setup and libsndfile calls to prevent audio corruption.

---

By carefully selecting these parameters, you can optimize **spirecordtodisk_ringbufferpausewin32** for your system’s performance characteristics and the requirements of your audio recording application.