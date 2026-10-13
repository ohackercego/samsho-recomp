# Samurai Shodown Sen (Edge of Destiny) — Native PC Static Recompilation

A native Windows x86_64 ahead-of-time (AOT) static recompilation of **Samurai Shodown Sen** (*Edge of Destiny* / サムライスピリッツ閃) for the Xbox 360 (Title ID: `534E07D5`, Media ID: `7DA5732F`), built using **ReXGlue** and **x360tools**.

Unlike conventional emulation (such as Xenia), this project translates the original PowerPC 3-core 6-thread Xenon machine code directly into native C++ ahead of time, compiling down to native x86_64 machine code. The result is zero JIT translation overhead, minimal input latency, native Direct3D 12 graphics presentation, and standalone execution without third-party emulator wrappers.

---

## Technical Architecture

* **AOT Static Binary Translation:** The game executable (`default.xex`) was unpacked and translated into ~24,569 optimized C++ functions via `x360tools`.
* **Guest Memory Virtualization:** Maps a 4GB guest virtual memory arena at base `0x0000000100000000` with direct address virtualization for game data, stack frames, and DMA structures.
* **Direct3D 12 GPU Pipeline:** Powered by `rexgpu-xenos.dll`, translating Xenos GPU draw packets, custom shader microcode, and EDRAM render target resolves directly to modern Direct3D 12 pipelines.
* **Low-Latency Audio:** XAudio2 and multi-channel XMA hardware audio decoding powered by SDL3 audio backends.
* **100% Native Subsystem:** Built as a native Windows GUI application (`WIN32` subsystem), executing cleanly without command-line windows or terminal console attachments.

---

## Reverse Engineering & Custom Overrides

### 1. Video Decoder Hardware Loop Bypass
The original Xbox 360 title depends on hardware-assisted video decoders for its opening logos and cinematic movies. In recompiled environments, missing video hardware interrupts caused the engine to freeze in an infinite decode poll loop:
* Overridden functions: `sub_821C4730`, `sub_821C54A0`, `sub_821C3B78`, `sub_821C4AB0`, `sub_821C6220`.
* The overrides correctly synthesize stream-completion state descriptors (`1280x720`, status code `0x28000002`), allowing the engine to transition instantly from startup into the title screen, menus, and real-time 3D battle scenes.

### 2. Menu Navigation via XInput Keystroke Emulation
The game's menu and UI dispatcher (`sub_82458188`) processes navigation commands strictly via Xbox 360 `XInputGetKeystroke` / `XamInputGetKeystrokeEx` (`sub_825A8B18`) rather than raw polled gamepad states:
* If this function returns anything other than `0` (`ERROR_SUCCESS`) or `4306` (`ERROR_EMPTY`), menu input is discarded completely.
* A custom queue-based keystroke engine was implemented in `src/overrides.cpp` with exact debouncing, key-repeat timing (initial hold: 280ms, repeat: 80ms), and release transitions (`KEYDOWN`, `REPEAT`, `KEYUP`), providing responsive menu navigation and option selection.

### 3. Dual-Player Keyboard Mapping & Controller Hooks
Both controller slots 0 (Player 1) and 1 (Player 2) report as active connected gamepads via `XamInputGetCapabilities`:
* **Player 1:** Arrow Keys + ASDF + QWERT + Enter / Space / Ctrl / Esc.
* **Player 2:** Number keys (compatible with both dedicated Numpads and compact laptop number rows).

---

## Controls Reference

### Player 1 (Keyboard)

| Key | Xbox 360 Input | Action / Menu Function |
| :--- | :--- | :--- |
| **Up Arrow** | D-Pad Up / Left Stick Up | Menu Up / Move Up |
| **Down Arrow** | D-Pad Down / Left Stick Down | Menu Down / Crouch |
| **Left Arrow** | D-Pad Left / Left Stick Left | Menu Left / Walk Back |
| **Right Arrow** | D-Pad Right / Left Stick Right | Menu Right / Walk Forward |
| **A** or **Space** | Button `A` | **Select / Confirm / Weak Slash (Horizontal)** |
| **S** | Button `B` | **Back / Cancel / Medium Slash (Vertical)** |
| **D** | Button `X` | Strong Slash / Special |
| **F** | Button `Y` | Kick |
| **Q** | `LB` (Left Shoulder) | Guard / Macro |
| **W** | `RB` (Right Shoulder) | Sidestep / Macro |
| **E** | `LT` (Left Trigger) | Special Trigger |
| **R** | `RT` (Right Trigger) | Rage Explosion / Trigger |
| **T** | `L3` (Left Thumb Click) | Taunt |
| **Enter** | `START` | **Start / Pause Game** |
| **Ctrl** or **Esc** | `BACK` | Select / Back |

---

### Player 2 (Number Keys — Numpad or Top Row)

| Key | Xbox 360 Input | Action / Function |
| :--- | :--- | :--- |
| **Numpad 8** or **8** | D-Pad Up / Left Stick Up | Jump / Up |
| **Numpad 2 / 5** or **2 / 5** | D-Pad Down / Left Stick Down | Crouch / Down |
| **Numpad 4** or **4** | D-Pad Left / Left Stick Left | Move Left |
| **Numpad 6** or **6** | D-Pad Right / Left Stick Right | Move Right |
| **Numpad 1** or **1** | Button `A` | Weak Slash / Confirm |
| **Numpad 3** or **3** | Button `B` | Medium Slash / Cancel |
| **Numpad 7** or **7** | Button `X` | Strong Slash |
| **Numpad 9** or **9** | Button `Y` | Kick |
| **Numpad 0** or **0** | `LB` (Left Shoulder) | Guard / Macro |
| **Numpad .** or **.** | `RB` (Right Shoulder) | Sidestep / Macro |
| **Numpad /** or **/** | `LT` (Left Trigger) | Special Trigger |
| **Numpad \*** | `RT` (Right Trigger) | Rage Explosion / Trigger |
| **Numpad +** or **+** | `START` | Start / Pause |
| **Numpad -** or **-** | `BACK` | Select / Back |

*Physical gamepads (Xbox controllers, DualSense, 8BitDo) connected to the PC are also automatically recognized via the SDL/XInput driver.*

---

## Directory Structure

```text
samsho-recomp/
├── extracted/                  # Game data extracted from Xbox 360 ISO
│   ├── default.xex
│   ├── ... (game assets, media, audio, stages)
├── rexglue-sdk/                # ReXGlue runtime SDK (headers & static libraries)
├── samsho_sen/                 # Recompiled C++ project
│   ├── generated/              # Ahead-of-time generated C++ translations
│   ├── src/
│   │   └── overrides.cpp       # Video decoder bypass stubs and input hooks
│   └── CMakeLists.txt
├── rexgpu-xenos.dll            # D3D12 Xenos translation backend
├── rexruntime.dll              # ReXGlue kernel, audio & guest memory runtime
├── samshosen.exe               # 100% Native Windows GUI executable
├── samshosen.toml              # Configuration file
└── run.bat                     # Quick launcher
```

---

## Requirements

* **Operating System:** Windows 10 (version 1903+) or Windows 11 64-bit.
* **Processor:** x86_64 CPU with AVX/AVX2 support (Intel Core 4th Gen+ or AMD Zen+).
* **Graphics:** DirectX 12 capable GPU (Direct3D feature level 11_0 or higher).
* **Audio:** Standard Windows audio device (DirectSound / WASAPI / XAudio2).

---

## Quick Start

1. Ensure the game assets have been extracted into the `extracted` folder alongside the executable.
2. Verify that `rexgpu-xenos.dll` and `rexruntime.dll` are located in the same directory as `samshosen.exe`.
3. Launch the game by executing:
   ```cmd
   run.bat
   ```
   Or directly:
   ```cmd
   samshosen.exe --game_data_root="extracted" --gpu_allow_invalid_fetch_constants=true
   ```

---

## Configuration (`samshosen.toml`)

Game resolution, display options, and logging can be tuned via `samshosen.toml`:

```toml
# Samurai Shodown Sen - ReXGlue Native PC Recompilation Config

game_data_root = "extracted"
protect_zero = false
gpu_plugin = "xenos"
vsync = true
window_width = 1280
window_height = 720
fullscreen = false
log_level = "info"
log_file = "samshosen_run.log"
gpu_allow_invalid_fetch_constants = true
```

---

## Building from Source

### Prerequisites
* **CMake** 3.25 or newer
* **Ninja** build system
* **Clang/LLVM** (Clang 17+) or MSVC (Visual Studio 2022 v143)
* Windows 10/11 SDK

### Build Commands
From the `samsho_sen` project directory:

```cmd
cmake --preset win-amd64-release
ninja -C out/build/win-amd64-release
```

Once the build finishes, copy the resulting binary:
```cmd
copy out\build\win-amd64-release\samshosen.exe ..\samshosen.exe
```

---

## Known Limitations

1. **FMV Cutscene Playback:**
   * Pre-rendered Bink/MPEG video intros and cinematic movies are currently bypassed via descriptor stubs to avoid hardware decoder stalls. In-engine 3D cutscenes, character intros, and victory sequences render natively in real time.
2. **Xbox Live & Network Multiplayer:**
   * Network multiplayer routines (`XamParty*`, Xbox Live matchmaking) are stubbed to return local offline mode. Local 2-Player Versus mode is fully operational.
3. **First-Run Shader Jitter:**
   * During the initial run, Direct3D 12 pipelines and translated shaders are compiled upon encountering new character visual effects. Compiled shaders are automatically persisted to the local cache (`%USERPROFILE%\Documents\samshosen\cache`) for smooth subsequent sessions.

---

## License & Credits

* Original Game & Intellectual Property: **SNK PLAYMORE**.
* Static Recompilation Tooling: **x360tools** & **ReXGlue** runtime by Tom Clay.
* Project Engineering: Ahead-of-time pipeline adaptations, video decoder interrupt overrides, and responsive keystroke input dispatching by **ANON** for **dj**.
