# Aimware 2016 - Full Reverse Engineering Report (Complete)

## 1. Binary Architecture Deep Dive

### 1.1 Dump Overview (from dump_analyzer.py)

| Dump | Base | Size | Entropy | Pointers | Prologues | Type |
|------|------|------|---------|----------|-----------|------|
| b34E10000 | 0x34E10000 | 192512 (0x2F000) | 7.317 | 3281 | 89 | CODE |
| b43AF0000 | 0x43AF0000 | 90112 (0x16000) | 5.951 | 2441 | 0 | DATA |
| b76ED0000 | 0x76ED0000 | 110592 (0x1B000) | 7.826 | 14 | 0 | STRING (encrypted) |
| b7C4A0000 | 0x7C4A0000 | 122880 (0x1E000) | 7.995 | 444 | 2 | CRT (encrypted) |

**Key findings:**
- CODE section has 89 function prologues at known addresses (0x34E1B6E0, 0x34E1D890, etc)
- DATA section contains 2441 cross-region pointers (1203 to DATA itself, 752 to STRING, 401 to CRT, 85 to CODE)
- STRING section high entropy 7.826 indicates XOR encryption with profile key
- CRT section entropy 7.995 near max, contains import table at 0x7C4B3E80
- Embedded MZ headers found at offsets: DATA+0x68B1, STRING+0x15BC, CRT+0x44E2 (possibly leftover PE)

### 1.2 Memory Layout (Complete)

```
0x34E10000 - 0x34E3F000 (192KB): CODE
  0x34E10000-0x34E1B000: Initialization & config
  0x34E1B000-0x34E1E000: Skinchanger (glove, paintkit)
  0x34E1E000-0x34E23000: ESP & Visuals
  0x34E23000-0x34E26000: Netvar proxies & misc
  0x34E25000-0x34E27000: Hooks (CreateMove, FrameStage, Paint, DrawModel, LockCursor)
  0x34E27000-0x34E2C000: Aimbot, AntiAim, Resolver
  0x34E2C000-0x34E32000: Skinchanger update, prediction
  0x34E32000-0x34E3A000: UI rendering (checkbox, slider, combobox, etc)
  0x34E3A000-0x34E3F000: Config & WndProc

0x43AF0000 - 0x43B06000 (90KB): DATA
  0x43AF0000-0x43AF7000: Global variables, config values, flags
  0x43AF7000-0x43AF8000: Render context, globals, skinchanger context pointers
  0x43AF8000-0x43AF9000: Config name buffer (32 bytes at 0x43AF8B2C)
  0x43AF9000-0x43AFA000: Weapon data, lag records pointer
  0x43AFA000-0x43AFB000: Convar storage
  0x43AFB000-0x43AFC000: Aimbot state, weapon state flags
  0x43AFC000-0x43AFD000: Menu state, skin active flag
  0x43AFD000-0x43AFE000: Sequence proxy storage
  0x43AFE000-0x43AFF000: Original function pointers, class pointers
  0x43AFF000-0x43B00000: Interface table & pattern scanned addresses
  0x43B00000-0x43B06000: Netvar proxy original storage, command tracking

0x76ED0000 - 0x76EEB000 (110KB): STRING (XOR encrypted)
  0x76ED0000-0x76ED9000: General strings, UI texts
  0x76ED9CE0: Glove model path (encrypted)
  0x76ED9EA0: Chams material (encrypted)
  0x76EDA000-0x76EEB000: More encrypted resources

0x7C4A0000 - 0x7C4BE000 (122KB): CRT & Helpers
  0x7C4A0000-0x7C4B3E80: CRT functions, helpers
  0x7C4B3E80-0x7C4B3F7C: Import table (reconstructed)
  0x7C4B3E80-0x7C4B3EA4: kernel32
  0x7C4B3EAC-0x7C4B3EE8: user32
  0x7C4B3EF4-0x7C4B3F44: msvcrt
  0x7C4B3F4C-0x7C4B3F7C: ntdll
```

### 1.3 Import Table Reconstruction (0x7C4B3E80)

From dump_analyzer and manual RE:

**kernel32.dll (100% accuracy):**
- 0x7C4B3E80: GetCurrentProcessId
- 0x7C4B3E84: GetFileSize
- 0x7C4B3E88: FindFirstFileW
- 0x7C4B3E8C: FindClose
- 0x7C4B3E90: FindNextFileW
- 0x7C4B3E94: GlobalLock
- 0x7C4B3E98: GlobalAlloc
- 0x7C4B3E9C: GlobalUnlock
- 0x7C4B3EA0: GlobalFree
- 0x7C4B3EA4: MultiByteToWideChar

**user32.dll (90% accuracy):**
- 0x7C4B3EAC: CloseClipboard
- 0x7C4B3EB0: IsClipboardFormatAvailable
- 0x7C4B3EB4: GetClipboardData
- 0x7C4B3EB8: GetCursorPos
- 0x7C4B3EBC: CallWindowProcA
- 0x7C4B3EC0: GetWindowTextA
- 0x7C4B3EC4: SetWindowLongW
- 0x7C4B3EC8: GetRawInputData
- 0x7C4B3ECC: ScreenToClient
- 0x7C4B3ED0: GetClientRect
- 0x7C4B3ED4: GetWindowThreadProcessId
- 0x7C4B3EE0: SetClipboardData
- 0x7C4B3EE4: OpenClipboard
- 0x7C4B3EE8: EmptyClipboard

**msvcrt.dll (100%):**
- 0x7C4B3EF4: tolower
- 0x7C4B3EF8: _vswprintf_c_l
- 0x7C4B3EFC: wcsncpy
- 0x7C4B3F00: strncpy
- 0x7C4B3F04: memset
- 0x7C4B3F08: sscanf
- 0x7C4B3F0C: sprintf
- 0x7C4B3F10: vswprintf
- 0x7C4B3F14: atoi
- 0x7C4B3F18: strchr
- 0x7C4B3F1C: strstr
- 0x7C4B3F20: _CIfmod
- 0x7C4B3F24: __libm_sse2_asinf
- 0x7C4B3F28: __libm_sse2_atan
- 0x7C4B3F2C: __libm_sse2_atan2
- 0x7C4B3F30: __libm_sse2_atanf
- 0x7C4B3F34: __libm_sse2_cosf
- 0x7C4B3F38: __libm_sse2_powf
- 0x7C4B3F3C: __libm_sse2_sinf
- 0x7C4B3F40: memcpy
- 0x7C4B3F44: toupper

**ntdll.dll (100%):**
- 0x7C4B3F4C: RtlLeaveCriticalSection
- 0x7C4B3F50: RtlEnterCriticalSection
- 0x7C4B3F54: NtQueryVirtualMemory
- 0x7C4B3F5C: NtReadFile
- 0x7C4B3F60: NtDeleteFile
- 0x7C4B3F64: NtClose
- 0x7C4B3F68: NtCreateFile
- 0x7C4B3F6C: RtlInitUnicodeString
- 0x7C4B3F70: NtWriteFile
- 0x7C4B3F74: RtlFreeHeap
- 0x7C4B3F78: NtDelayExecution
- 0x7C4B3F7C: RtlAllocateHeap

### 1.4 Interface Table (0x43AFF000)

| Address | Module | Interface | Purpose |
|---------|--------|-----------|---------|
| 0x43AFF050 | engine.dll | VEngineClient013 | GetLocalPlayer, IsInGame, etc |
| 0x43AFF01C | engine.dll | EngineTraceClient004 | TraceRay, ClipTrace |
| 0x43AFF04C | engine.dll | VModelInfoClient004 | GetModel, FindMDL |
| 0x43AFF088 | engine.dll | VDebugOverlay004 | WorldToScreen |
| 0x43AFF094 | client.dll | VClient018 | GetAllClasses |
| 0x43AFEFDC | client.dll | VClientEntityList003 | GetClientEntity |
| 0x43AFF0A8 | client.dll | VClientPrediction001 | RunCommand |
| 0x43AFF0AC | client.dll | GameMovement001 | Movement |
| 0x43AFF000 | materialsystem.dll | VMaterialSystem080 | FindMaterial |
| 0x43AFF098 | vguimatsurface.dll | VGUI_Surface031 | Drawing |
| 0x43AFF028 | vphysics.dll | VPhysicsSurfaceProps001 | Surface data |
| 0x43AFF030 | vstdlib.dll | RandomFloat | Random |
| 0x43AFF054 | vstdlib.dll | RandomInt | Random |
| 0x43AFF03C | vstdlib.dll | RandomSeed | Random |
| 0x43AFF044 | vstdlib.dll | VEngineCvar007 | Convars |
| 0x43AFF048 | localize.dll | Localize_ | Localization |
| 0x43AFF0B0 | datacache.dll | MDLCache | Model cache |

## 2. Function Deep Dive

### 2.1 ClientMode::CreateMove (0x34E258A0)

**Purpose:** Main tick, aimbot, antiaim, fakelag

**Flow:**
1. Check `0x43AFBEA2` (aimbot enabled)
2. Check weapon state at `0x43AFCD99`
3. Call original CreateMove
4. Store command number at `0x43AFE3D8[cmd_number]`
5. Call hitchance calc `0x34E282D0`
6. Get best target `0x34E28D40`
7. Calculate angle, apply silent aim
8. Process antiaim `0x34E2A910`, `0x34E2AF90`
9. Process fakelag `0x34E2B720`
10. Return false for silent aim

**Decompiled:**
```cpp
bool __stdcall hkCreateMove(float frametime, CUserCmd* cmd) {
    if (!cmd || !cmd->command_number) return false;
    if (!*(bool*)0x43AFBEA2) return original(frametime, cmd);
    
    QAngle oldAngles = cmd->viewangles;
    Vector target;
    int idx = GetBestTarget(cmd, target);
    if (idx != -1) {
        QAngle aim = CalcAngle(localEye, target);
        if (CalculateHitchance(cmd, aim, 50.0f)) {
            cmd->viewangles = aim;
            cmd->buttons |= IN_ATTACK;
            FixMovement(cmd, oldAngles);
        }
    }
    
    // AntiAim
    QAngle aa = CalculateAntiAim(cmd, send_packet);
    cmd->viewangles = aa;
    
    return false;
}
```

### 2.2 FrameStageNotify (0x34E26330)

**Purpose:** NoSmoke, SkinChanger, Thirdperson

**Stages:**
- `FRAME_NET_UPDATE_POSTDATAUPDATE_START (4)`: Skinchanger `0x34E2C580`
- `FRAME_RENDER_START`: Resolver `0x34E25040`
- Every stage: Check IsInGame, get local player

**NoSmoke:**
```cpp
*(int*)*(int*)0x43AFF0B4 = 0; // Smoke count = 0
```

### 2.3 Paint (0x34E26660)

**Purpose:** ESP rendering

```cpp
void __fastcall hkPaint(void* ecx, int mode) {
    original(ecx, mode);
    if (mode & 1) { // PAINT_UIPANELS
        DrawESP(); // 0x34E1D1D0
    }
}
```

**DrawESP (0x34E1D1D0):**
- Iterate 1..64 entities via `0x43AFEFDC` (EntityList)
- Check dormant, health, team
- WorldToScreen via `0x43AFF088` (DebugOverlay)
- Draw box (2D/3D/corners)
- Draw health bar
- Draw name, weapon
- Draw skeleton via bone matrix
- Draw Aimware menu

### 2.4 DrawModel (0x34E25960) - Chams

**Purpose:** Material override for visible/invisible

```cpp
void __fastcall hkDrawModel(...) {
    if (!*(bool*)0x43AFCCB2) return original(...);
    
    // Get profile XOR key at 0x43AFF218
    int key = *(int*)*(int*)0x43AFF218;
    // Decrypt material string at 0x76ED9EA0
    char* matName = Decrypt(0x76ED9EA0, key);
    
    IMaterial* mat = MaterialSystem->FindMaterial(matName);
    mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
    mat->ColorModulate(color);
    
    // Draw hidden
    original(...);
    
    // Draw visible
    mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
    original(...);
}
```

### 2.5 LockCursor (0x34E26620)

**Purpose:** Unlock mouse when menu open

```cpp
void __fastcall hkLockCursor(void* ecx) {
    if (*(bool*)0x43AFD094) { // menu open?
        *(bool*)0x43AFE638 = true;
        Surface->UnlockCursor(); // vtable[66]
        return;
    }
    *(bool*)0x43AFE638 = false;
    original(ecx);
}
```

### 2.6 Netvar Proxies

**Pitch Proxy (0x34E24BF0):**
```cpp
void __cdecl PitchProxy(RecvProxyData& data, void* ent, void* out) {
    int idx = GetEntityIndex(ent);
    LagRecord* rec = (LagRecord*)(*(int*)0x43AF7A84 + (idx-1)*0x3234);
    rec->eye_angles.pitch = data.value._float;
    // Clamp
    if (data.value._float > 89) data.value._float = 89;
    if (data.value._float < -89) data.value._float = -89;
    *(float*)out = data.value._float;
}
```

**Yaw Proxy (0x34E24D20):** Similar, stores at 0x3204, applies resolver

**LBY Proxy (0x34E24EC0):**
```cpp
void __cdecl LBYProxy(RecvProxyData& data, void* ent, void* out) {
    LagRecord* rec = GetLagRecord(idx);
    float old = rec->lower_body_yaw;
    rec->lower_body_yaw = data.value._float;
    rec->lby_update_time = curtime;
    
    if (fabs(data.value._float - old) > 35.0f) {
        // LBY update detected
    }
    
    *(float*)out = data.value._float;
}
```

## 3. Config System (0x34E34746, 0x34E357C5, 0x34E35983)

**Path:** `C:\aimware\` via `L"\\??\\C:\\aimware\\"` (NT path)

**GenConfigPath (0x34E34E90):**
```cpp
void __stdcall GenConfigPath(const char* name, wchar_t* out) {
    wcscpy(out, L"\\??\\C:\\aimware\\");
    // Append name, sanitize
    for (int i=0; name[i] && i<255; i++) out[wcslen(out)] = name[i];
}
```

**GetConfigList (0x34E35983):**
Uses `FindFirstFileW`, `FindNextFileW`, `FindClose` at import table
Scans `C:\aimware\*.cfg`

**DumpConfig (0x34E357C5):**
Serializes binary config with XOR

**ConfigAction (0x34E34746):**
Handles save/load/delete via UI

## 4. Skinchanger (0x34E2C580, 0x34E1DFC9, 0x34E1E2DA)

**OnPostDataUpdate (0x34E2C580):**
Called at stage 4, updates skins

**GloveChanger (0x34E1DFC9):**
```cpp
// Decrypt glove model path
char* enc = (char*)0x76ED9CE0;
int key = *(int*)*(int*)0x43AFF218; // profile xor_key
for (int i=0; enc[i]; i++) {
    enc[i] ^= key; // XOR decrypt
}
// Find model via MDLCache::FindMDL
// Apply to m_hMyWearables
```

**PaintkitChanger (0x34E1E2DA):**
- Parse paintkit ID from config
- Override `m_nFallbackPaintKit` (0x43AFEF04)
- `m_flFallbackWear` (0x43AFEF2C)
- `m_nFallbackSeed` (0x43AFEF28)
- `m_szCustomName` (0x43AFEF10)

**Force Update:**
Via `cl_fullupdate` command at `0x43AFF058`

## 5. XOR Protection

**Mechanism:** Critical branches check XOR decrypted value, if fails -> JZ to exit

**Patches:** Set byte to 0x75 (JNZ) to bypass

**List (56 addresses):**
- Config: 0x34E1EAAD, 0x34E1EB16, 0x34E1EB86, 0x34E2B4BB, 0x34E2B473
- ESP: 0x34E1FE4A, 0x34E20169, 0x34E208A5, 0x34E20995, 0x34E20A8A, 0x34E20BC6, 0x34E20D55, 0x34E20E2F
- Skinchanger: 0x34E1DFC9, 0x34E1E2DA, 0x34E1E32A, 0x34E1E38E, 0x34E1F14E
- Chams: 0x34E25A13
- Spam: 0x34E2AD02, 0x34E2AC29
- UI: 0x34E37D73, 0x34E38185, 0x34E388A4, 0x34E38FB2, 0x34E39396, etc

**Loader bypass:**
```cpp
for (auto addr : xor_patches) {
    *(BYTE*)addr = 0x75; // JNZ
}
```

## 6. Anti-Aim & Resolver

**Pitch (0x34E2A910):**
- Emotion: 89°
- Up: -89°
- Zero: 0°
- Fake: 180°

**Yaw (0x34E2AF90):**
- Backward: 180°
- Sideways: 90°
- Spinbot: spinning via `curtime * speed`
- Jitter: 90° / -90° flip

**Fakelag (0x34E2B720):**
```cpp
static int choked = 0;
choked++;
if (choked >= 6) { choked = 0; send_packet = true; }
else send_packet = false;
```

**Resolver:**
- LBY: Use LBY when moving, last moving LBY when standing
- Bruteforce: Cycle LBY, LBY+90, LBY-90, LBY+180 on missed shots
- Advanced: Combine movement check + LBY update detection

**Lag Records (0x43AF7A84):**
- Allocated: `0x3234 * 64` = 0xC8D00 bytes (819KB)
- Per player: 0x3234 bytes
- Contains: origin, velocity, mins/maxs, eye_angles (0x3200 pitch, 0x3204 yaw), LBY (0x320C), simtime, flags, bone_matrix[128], valid

## 7. Advanced Mapper (New)

**Features:**
- Fixed address allocation with `VirtualQuery` check + `NtUnmapViewOfSection` cleanup
- Entropy analysis to detect encryption
- Pointer scanning to verify data section
- PE parsing to find embedded MZ/PE
- X86 prologue scanning (89 found in CODE)
- String extraction (601 in CODE, 199 in DATA, 1120 in STRING, 535 in CRT)
- Relocation handling for cross-region pointers
- Protection finalization (CODE->RX, DATA->RW, STRING->R)

**Usage:**
```cpp
AdvancedMapper::Instance().AddRegion(0x34E10000, 192512, "CODE", b34E10000, PAGE_EXECUTE_READWRITE, true);
AdvancedMapper::Instance().AddRegion(0x43AF0000, 90112, "DATA", b43AF0000, PAGE_READWRITE, false);
AdvancedMapper::Instance().MapAll(true);
```

## 8. Conclusion

Aimware 2016 is a fully functional internal cheat with:
- **Aimbot**: FOV, hitchance (256 seeds), autowall, multipoint, backtrack via lag records
- **Visuals**: ESP 2D/3D/box, health, name, weapon, skeleton, chams (ignorez, flat), NoSmoke (smoke count=0), NoFlash
- **Misc**: Skinchanger (glove + paintkit via XOR strings), config system at C:\aimware\, chat/name spam, bunnyhop, thirdperson
- **Protection**: XOR checks bypassed via JNZ patches

The dump loader in aw.cpp successfully executes it by:
1. Mapping fixed addresses
2. Fixing imports (kernel32, user32, msvcrt, ntdll)
3. Resolving engine interfaces
4. Pattern scanning for functions
5. Fixing convars
6. Allocating lag records
7. Patching XOR
8. Hooking via VMT

All addresses documented in `reversed/memory_map.hpp`, `offsets.hpp`, `functions.hpp`
