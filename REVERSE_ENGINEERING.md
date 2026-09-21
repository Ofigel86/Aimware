# Full Reverse Engineering Report: Aimware 2016 Binary Dump

## 1. Executive Summary & Binary Architecture

This repository contains a memory dump loader and wrapper (`aimware2016.sln`) designed to reconstruct and execute an x86 binary memory dump of **Aimware CS:GO (2016 Build)**.

The core cheat binary is stored in 4 un-relocated memory dumps:
* `b34E10000.h` (`0x34E10000`, Size: 192,512 bytes) — Code section (x86 assembly execution block).
* `b43AF0000.h` (`0x43AF0000`, Size: 90,112 bytes) — Data section (Global state, interface pointers, offsets, config values).
* `b76ED0000.h` (`0x76ED0000`, Size: 110,592 bytes) — String & XOR encrypted resource section.
* `b7C4A0000.h` (`0x7C4A0000`, Size: 122,880 bytes) — CRT & helper function section.

---

## 2. Fixed Memory Map & Pointer Table

| Address Range | Component | Description |
|---|---|---|
| `0x34E10000` - `0x34E3F000` | Code Section | x86 machine code containing hooks, rendering, skinchanger, UI, and logic |
| `0x43AF0000` - `0x43B06000` | Data Section | Interface table, netvar offsets, convar pointers, lag records base pointer |
| `0x43AF7A84` | Lag Records Base | Pointer to allocated `0x3234 * 64` bytes buffer for player history records |
| `0x43AFF218` | Profile Context | Pointer to `profile_t` structure containing config path `C:\aimware\` and XOR key |
| `0x7C4B3E80` - `0x7C4B3F7C` | Import Table | Reconstructed Win32 API function addresses |

---

## 3. Reconstructed Import Table (`0x7C4B3E80`)

The dump calls external DLL functions through a manually reconstructed import table:

* **Kernel32.dll**:
  * `0x7C4B3E80`: `GetCurrentProcessId`
  * `0x7C4B3E84`: `GetFileSize`
  * `0x7C4B3E88`: `FindFirstFileW`
  * `0x7C4B3E8C`: `FindClose`
  * `0x7C4B3E90`: `FindNextFileW`
  * `0x7C4B3E94`..`0x7C4B3EA0`: `GlobalLock`, `GlobalAlloc`, `GlobalUnlock`, `GlobalFree`
  * `0x7C4B3EA4`: `MultiByteToWideChar`

* **User32.dll**:
  * `0x7C4B3EAC`..`0x7C4B3EB4`: `CloseClipboard`, `IsClipboardFormatAvailable`, `GetClipboardData`
  * `0x7C4B3EB8`: `GetCursorPos`
  * `0x7C4B3EBC`: `CallWindowProcA`
  * `0x7C4B3EC0`: `GetWindowTextA`
  * `0x7C4B3EC4`: `SetWindowLongW`
  * `0x7C4B3ECC`: `ScreenToClient`
  * `0x7C4B3ED0`: `GetClientRect`
  * `0x7C4B3EE0`..`0x7C4B3EE8`: `SetClipboardData`, `OpenClipboard`, `EmptyClipboard`

* **MSVCRT.dll**:
  * `0x7C4B3EF4`: `tolower`
  * `0x7C4B3EF8`: `_vswprintf_c_l`
  * `0x7C4B3EFC`: `wcsncpy`
  * `0x7C4B3F00`: `strncpy`
  * `0x7C4B3F04`: `memset`
  * `0x7C4B3F08`: `sscanf`
  * `0x7C4B3F0C`: `sprintf`
  * `0x7C4B3F14`: `atoi`
  * `0x7C4B3F18`: `strchr`
  * `0x7C4B3F1C`: `strstr`
  * Math intrinsics: `sinf`, `cosf`, `powf`, `atan2`, `asinf`

* **NTDLL.dll**:
  * `0x7C4B3F4C`: `RtlLeaveCriticalSection`
  * `0x7C4B3F50`: `RtlEnterCriticalSection`
  * `0x7C4B3F54`: `NtQueryVirtualMemory`
  * `0x7C4B3F58`: `NtTerminateProcess`
  * `0x7C4B3F5C`..`0x7C4B3F70`: `NtReadFile`, `NtDeleteFile`, `NtClose`, `NtCreateFile`, `NtWriteFile`
  * `0x7C4B3F74`..`0x7C4B3F7C`: `RtlFreeHeap`, `NtDelayExecution`, `RtlAllocateHeap`

---

## 4. Reconstructed Source Engine Interface Map (`0x43AFF000`)

The cheat queries engine interfaces and writes pointers to fixed addresses:

| Fixed Address | Interface / Module | Function / Interface Name |
|---|---|---|
| `0x43AFF050` | `engine.dll` | `VEngineClient013` |
| `0x43AFF01C` | `engine.dll` | `EngineTraceClient004` |
| `0x43AFF04C` | `engine.dll` | `VModelInfoClient004` |
| `0x43AFF088` | `engine.dll` | `VDebugOverlay004` |
| `0x43AFF094` | `client.dll` | `VClient018` |
| `0x43AFEFDC` | `client.dll` | `VClientEntityList003` |
| `0x43AFF0A8` | `client.dll` | `VClientPrediction001` |
| `0x43AFF0AC` | `client.dll` | `GameMovement001` |
| `0x43AFF000` | `materialsystem.dll` | `VMaterialSystem080` |
| `0x43AFF098` | `vguimatsurface.dll` | `VGUI_Surface031` |
| `0x43AFF028` | `vphysics.dll` | `VPhysicsSurfaceProps001` |
| `0x43AFF044` | `vstdlib.dll` | `VEngineCvar007` |

---

## 5. Disassembly Analysis of Key Binary Entry Points

### 5.1 `ClientMode::CreateMove` (`0x34E258A0`)
```assembly
34e258a0:  push ebp
34e258a1:  mov ebp, esp
34e258a4:  cmp byte ptr [0x43afbea2], 0   ; Check Aimbot enabled flag
34e258ab:  je 0x34e258b6
34e258ad:  cmp byte ptr [0x43afcd99], 2   ; Check active weapon state
34e258b4:  jne 0x34e258ca
34e258b6:  push [ebp+0xC]                 ; Push CUserCmd pointer
34e258b9:  movss xmm0, [ebp+0x8]          ; Push input_sample_frametime
34e258c4:  call dword ptr [0x43afe178]    ; Call original ClientMode::CreateMove
...
34e2590c:  mov [0x43afe3d8 + edx*4], eax  ; Store command number in tracking array
34e25918:  call 0x34e282d0                ; Execute Hitchance & Spread seed calculation
```
* **Function Purpose**: Main tick entry point. Intercepts `CUserCmd`, evaluates Aimbot/Anti-Aim/UserCmd flags, calls seed calculation `0x34E282D0`, and applies view angle adjustments.

---

### 5.2 `CHLClient::FrameStageNotify` (`0x34E26330`)
```assembly
34e26330:  push ebp
34e26331:  mov ebp, esp
34e2633c:  mov ecx, [0x43aff050]          ; Get VEngineClient
34e26344:  mov eax, [eax+0x30]            ; Call IsInGame()
34e26347:  call eax
34e26349:  mov ecx, [0x43afefdc]          ; Get VClientEntityList
34e26352:  call dword ptr [edx+0xC]       ; Get local player entity
...
34e263a1:  mov eax, [0x43aff0b4]          ; Get smoke count address
34e263a6:  mov dword ptr [eax], 0         ; NoSmoke: zero out active smoke count!
...
34e263f3:  cmp ebx, 4                     ; Check stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START
34e263fc:  call 0x34E2C580                ; Execute SkinChanger update sequence
```
* **Function Purpose**: Called every frame stage.
  * **NoSmoke**: Overwrites global smoke count at `0x43AFF0B4` to 0.
  * **SkinChanger**: Calls `0x34E2C580` at stage 4 (`FRAME_NET_UPDATE_POSTDATAUPDATE_START`).
  * **ThirdPerson / View Angles**: Updates local player thirdperson recoil angle state.

---

### 5.3 `EngineVGUI::Paint` (`0x34E26660`)
```assembly
34e26660:  push ebx
34e26661:  mov ebx, [esp+0x8]             ; Get mode flags
34e26666:  call dword ptr [0x43afe63c]    ; Call original EngineVGUI::Paint
34e2666c:  test bl, 1                     ; Test PAINT_UIPANELS flag
34e2666f:  je 0x34e267c3
34e26675:  mov ecx, [0x43af7704]          ; Get global Aimware render context
34e2667b:  call 0x34e1d1d0                ; Execute ESP & GUI drawing loop
```
* **Function Purpose**: Main rendering loop. Calls original `EngineVGUI::Paint`, then checks if UI panels are being rendered and invokes `0x34E1D1D0` to draw 2D/3D ESP, Health bars, Skeleton bones, and Aimware GUI.

---

### 5.4 `StudioRender::DrawModel` (`0x34E25960`)
```assembly
34e25960:  sub esp, 0x6c
34e25963:  cmp byte ptr [0x43afccb2], 0   ; Check skin_active / chams flag
34e25978:  je 0x34e25b5a                  ; Skip to original DrawModel if disabled
34e2598b:  mov ecx, [ebp+0x18]            ; Get renderable info
34e259b3:  mov ecx, [0x43aff050]          ; Get VEngineClient
34e259c2:  call edx                       ; GetLocalPlayer
...
34e25a0b:  mov eax, [0x43aff218]          ; Get profile_t XOR context
34e25a15:  mov cl, [0x76ed9ea0]           ; Load Chams material override string
```
* **Function Purpose**: Intercepts player model rendering for Chams. Applies material parameters (ignorez / flat / wireframe) and sets hidden/visible colors.

---

### 5.5 `Surface::LockCursor` (`0x34E26620`)
```assembly
34e26620:  mov eax, [0x43afe638]
34e26636:  cmp byte ptr [0x43afd094], 0   ; Check menu_open flag
34e2663d:  je 0x34e2664c
34e2663f:  mov byte ptr [eax], 1          ; Enable unlocked cursor
34e26642:  mov eax, [ecx]
34e26644:  mov eax, [eax+0x108]           ; Call ISurface::UnlockCursor()
34e2664a:  jmp eax
34e2664c:  mov byte ptr [eax], 0
34e2664f:  jmp dword ptr [0x43afe634]     ; Call original LockCursor
```
* **Function Purpose**: When the Aimware menu is open (`0x43AFD094 == 1`), diverts `LockCursor` to `ISurface::UnlockCursor()` so the user can move the mouse cursor across the GUI.

---

### 5.6 Netvar Proxy Callbacks
* **`0x34E24BF0` (`CCSPlayer::m_angEyeAngles[0]`)**:
  * Queries `IVEngineClient::GetLocalPlayer()` (`vtable[0x138/4 = 78]`).
  * Calculates player lag record address: `records_base (0x43AF7A84) + ent_index * 0x3234`.
  * Stores pitch angle into lag record offset `0x3200`.

* **`0x34E24D20` (`CCSPlayer::m_angEyeAngles[1]`)**:
  * Stores yaw angle into lag record offset `0x3204`.

* **`0x34E24EC0` (`CCSPlayer::m_flLowerBodyYawTarget`)**:
  * Stores LBY angle into lag record offset `0x320C` and updates delta timer at `0x3210`.

---

### 5.7 GloveChanger (`0x34E1DFC9`) & PaintkitChanger (`0x34E1E2DA`)
```assembly
34e1dfc9:  je 0x34e1dff6
34e1dfcb:  mov cl, [0x76ed9ce0]           ; Load XOR encrypted glove model path
34e1dfd1:  mov eax, [0x43aff218]          ; Load profile XOR key
34e1dfd6:  xor cl, [eax]                  ; Decrypt character byte
...
34e1e003:  call strstr                    ; Find weapon model string match
34e1e026:  call strchr                    ; Split model extension
34e1e046:  call eax                       ; Call MDLCache::FindMDL (0x43AFF04C)
```
* **Function Purpose**: Decrypts weapon and glove model strings using the profile key stored in `profile_t` (`0x43AFF218`), matches item IDs using `strstr`/`strchr`, and forces custom MDL model loading via `MDLCache`.

---

## 6. XOR Obfuscation & Protection Patches

The binary dump employs XOR checks on critical function branches to prevent execution if altered or inspected. The loader neutralizes these checks by setting byte opcodes to `0x75` (`JNZ` instruction):

```cpp
std::vector<DWORD> xor_patches = {
    // Config Save & Parsing
    0x34E1EAAD, 0x34E1EB16, 0x34E1EB86, 0x34E2B4BB, 0x34E2B473,

    // ESP & Visuals
    0x34E1FE4A, 0x34E20169, 0x34E208A5, 0x34E20995,
    0x34E20A8A, 0x34E20BC6, 0x34E20D55, 0x34E20E2F,

    // SkinChanger & GloveChanger
    0x34E1DFC9, 0x34E1E2DA, 0x34E1E32A, 0x34E1E38E, 0x34E1F14E,

    // DrawModel Chams
    0x34E25A13,

    // Chat & Name Spam
    0x34E2AD02, 0x34E2AC29,

    // UI Elements
    0x34E37D73, 0x34E38185, 0x34E388A4, 0x34E38FB2, 0x34E39396
};
```

---

## 7. Conclusion

The Aimware 2016 binary dump is a fully functional 32-bit x86 internal cheat executable. By mapping fixed addresses `0x34E10000`, `0x43AF0000`, `0x76ED0000`, and `0x7C4A0000`, resolving Win32 imports and Source Engine interfaces, and patching XOR branches, the loader in `aw.cpp` successfully executes the compiled Aimware binary directly in memory.
