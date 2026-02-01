# On-Screen Text and Timer Overlay (spitext) – Purpose and Basic Behavior

## Purpose

The **spitext** module provides a transparent, always-on-top Win32 window that renders arbitrary text or live timer counters over the desktop or another target window. It serves two main goals:

- Offer a **generic overlay utility** that can be embedded into any Win32 application.
- Seamlessly integrate with the recorder to display recording status, elapsed time, countdowns, or clocks without obscuring underlying content.

## Basic Behavior

Upon launch, `_tWinMain` parses command-line parameters to configure display text, counter modes, positioning, and timing. It then:

1. Registers a custom window class with **layered** and **transparent** styles.
2. Creates a borderless, top-most popup window.
3. Applies a **color-keyed** transparency (RGB(255,0,255)) and alpha blending.
4. Enters the standard message loop, handling:
5. **WM_TIMER** to update timestamps and trigger redraws.
6. **WM_PAINT** to draw text or formatted counters via XOR-blitting.
7. Cleans up on **WM_DESTROY**, terminating the overlay.

## 🎯 Key Features

- **Arbitrary Text & Timers**: Display any string or live counters at runtime.
- **Multiple Counter Modes**

| Mode | Identifier | Description |
| --- | --- | --- |
| COUNTUP | `global_countermodeCOUNTUP` | Counts up from a specified start. |
| COUNTDOWN | `global_countermodeCOUNTDOWN` | Counts down toward zero. |
| CLOCK | `global_countermodeCLOCK` | Shows local time (HH:MM:SS). |


(Configured via `global_spitextstring` .)

| Format | Identifier | Example |
| --- | --- | --- |
| HH:MM:SS | `global_counterformatHHMMSS` | 02:15:30 |
| HH:MM | `global_counterformatHHMM` | 02:15 |
| MM:SS | `global_counterformatMMSS` | 15:30 |
| HH / MM / SS | `global_counterformatHH/MM/SS` | 02 or 15 or 30 |


- **Format Flexible**
- **Positioning Options**
- Absolute or **relative** to a monitor, window handle, class, or title.
- **Justification** (top/center/bottom, left/center/right).
- **Force-fit** font scaling within a region.
- **Transparent Overlay**
- Uses a **layered window** (`WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT`) and a **color-key** of RGB(255,0,255) .
- **XOR blitting** erases previous text before drawing updated frames via `DrawTextXOR` .

## Architecture and Components

### Entry Point: `_tWinMain`

Parses arguments into globals (`global_spitextstring`, `global_x`, `global_y`, `global_starttime_sec`, etc.) and initializes timers and ring buffer hooks if integrated with the recorder .

```cpp
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
{
    // Parse command-line into global_* variables
    global_startstamp_ms = GetTickCount();
    // ... push global_spitextstring, modes, formats ...
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;
    // Standard message loop...
}
```

### Window Class Registration

```cpp
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = {};
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.hInstance     = hInstance;
    wcex.hbrBackground = CreateSolidBrush(global_keyingcolor);
    wcex.lpszClassName = szWindowClass;
    return RegisterClassEx(&wcex);
}
```

### Instance Initialization: `InitInstance`

Creates the overlay window with extended styles:

```cpp
DWORD Flags1 = WS_EX_COMPOSITED
             | WS_EX_LAYERED
             | WS_EX_TOPMOST
             | WS_EX_TRANSPARENT;
HWND hWnd = CreateWindowEx(
    Flags1,
    szWindowClass,
    szTitle,
    WS_POPUP,
    /*x=*/global_x, /*y=*/global_y,
    /*w=*/100, /*h=*/100,
    nullptr, nullptr, hInstance, nullptr
);
SetLayeredWindowAttributes(hWnd, global_keyingcolor, global_alpha, LWA_COLORKEY);
```

This ensures the window never grabs focus and blends seamlessly .

### Message Handling: `WndProc`

- **WM_TIMER**: Updates `global_nowstamp_ms`, computes `global_timetodisplay_sec`, invalidates the client rect for repainting.
- **WM_PAINT**:
- BeginPaint/EndPaint.
- Select font `global_hFont`.
- Format text or timer string based on `global_textmode` & `global_textformat`.
- Erase previous string via XOR mode.
- Draw new string at `(global_x, global_y)` .
- **WM_KEYDOWN**: Toggles pause/resume if integrated (P-key).
- **WM_DESTROY**: Cleans up resources and quits.

## Integration and Dependencies

- Depends on **Win32 API** (`windows.h`, `tchar.h`, `ShellAPI.h`) and **GDI** services.
- Leverages **spiutility** for monitor and window enumeration (struct `EnumWindowsStruct_spitext`).
- Can share globals and headers with the recorder module for synchronized start/stop triggers.
- Compiled as part of the main executable—no separate DLL or EXE required.

## Card Block: Best Practice

```card
{
    "title": "Performance Impact",
    "content": "Minimize redraw regions and timer granularity to reduce CPU usage."
}
```

---

This detailed documentation covers the **purpose**, **behavior**, **components**, and **integration points** of the spitext module, illustrating how it delivers a configurable, transparent overlay for both text and timer displays in a Windows C++ application.