#include "aw.h"
#include "detours.h"

#define CSGO2016

using namespace Aimware;
using namespace Aimware::Utils;

bool call_in_bounds(void* addr) {
    uintptr_t u = (uintptr_t)addr;
    return u > 0x34E10000 && u < (0x34E10000 + sizeof(b34E10000));
}

// ===================== Import Table =====================
std::unordered_map<DWORD, std::pair<const char*, const char*>> imports = {
    { 0x7C4B3E80, { "kernel32.dll", "GetCurrentProcessId" } },
    { 0x7C4B3E84, { "kernel32.dll", "GetFileSize" } },
    { 0x7C4B3E88, { "kernel32.dll", "FindFirstFileW" } },
    { 0x7C4B3E8C, { "kernel32.dll", "FindClose" } },
    { 0x7C4B3E90, { "kernel32.dll", "FindNextFileW" } },
    { 0x7C4B3E94, { "kernel32.dll", "GlobalLock" } },
    { 0x7C4B3E98, { "kernel32.dll", "GlobalAlloc" } },
    { 0x7C4B3E9C, { "kernel32.dll", "GlobalUnlock" } },
    { 0x7C4B3EA0, { "kernel32.dll", "GlobalFree" } },
    { 0x7C4B3EA4, { "kernel32.dll", "MultiByteToWideChar" } },

    { 0x7C4B3EAC, { "user32.dll", "CloseClipboard" } },
    { 0x7C4B3EB0, { "user32.dll", "IsClipboardFormatAvailable" } },
    { 0x7C4B3EB4, { "user32.dll", "GetClipboardData" } },
    { 0x7C4B3EB8, { "user32.dll", "GetCursorPos" } },
    { 0x7C4B3EBC, { "user32.dll", "CallWindowProcA" } },
    { 0x7C4B3EC0, { "user32.dll", "GetWindowTextA" } },
    { 0x7C4B3EC4, { "user32.dll", "SetWindowLongW" } },
    { 0x7C4B3EC8, { "user32.dll", "GetRawInputData" } },
    { 0x7C4B3ECC, { "user32.dll", "ScreenToClient" } },
    { 0x7C4B3ED0, { "user32.dll", "GetClientRect" } },
    { 0x7C4B3ED4, { "user32.dll", "GetWindowThreadProcessId" } },
    { 0x7C4B3EE0, { "user32.dll", "SetClipboardData" } },
    { 0x7C4B3EE4, { "user32.dll", "OpenClipboard" } },
    { 0x7C4B3EE8, { "user32.dll", "EmptyClipboard" } },

    { 0x7C4B3EF4, { "msvcrt.dll", "tolower" } },
    { 0x7C4B3EF8, { "msvcrt.dll", "_vswprintf_c_l" } },
    { 0x7C4B3EFC, { "msvcrt.dll", "wcsncpy" } },
    { 0x7C4B3F00, { "msvcrt.dll", "strncpy" } },
    { 0x7C4B3F04, { "msvcrt.dll", "memset" } },
    { 0x7C4B3F08, { "msvcrt.dll", "sscanf" } },
    { 0x7C4B3F0C, { "msvcrt.dll", "sprintf" } },
    { 0x7C4B3F10, { "msvcrt.dll", "vswprintf" } },
    { 0x7C4B3F14, { "msvcrt.dll", "atoi" } },
    { 0x7C4B3F18, { "msvcrt.dll", "strchr" } },
    { 0x7C4B3F1C, { "msvcrt.dll", "strstr" } },
    { 0x7C4B3F20, { "msvcrt.dll", "_CIfmod" } },
    { 0x7C4B3F24, { "msvcrt.dll", "__libm_sse2_asinf" } },
    { 0x7C4B3F28, { "msvcrt.dll", "__libm_sse2_atan" } },
    { 0x7C4B3F2C, { "msvcrt.dll", "__libm_sse2_atan2" } },
    { 0x7C4B3F30, { "msvcrt.dll", "__libm_sse2_atanf" } },
    { 0x7C4B3F34, { "msvcrt.dll", "__libm_sse2_cosf" } },
    { 0x7C4B3F38, { "msvcrt.dll", "__libm_sse2_powf" } },
    { 0x7C4B3F3C, { "msvcrt.dll", "__libm_sse2_sinf" } },
    { 0x7C4B3F40, { "msvcrt.dll", "memcpy" } },
    { 0x7C4B3F44, { "msvcrt.dll", "toupper" } },

    { 0x7C4B3F4C, { "ntdll.dll", "RtlLeaveCriticalSection" } },
    { 0x7C4B3F50, { "ntdll.dll", "RtlEnterCriticalSection" } },
    { 0x7C4B3F54, { "ntdll.dll", "NtQueryVirtualMemory" } },
    { 0x7C4B3F5C, { "ntdll.dll", "NtReadFile" } },
    { 0x7C4B3F60, { "ntdll.dll", "NtDeleteFile" } },
    { 0x7C4B3F64, { "ntdll.dll", "NtClose" } },
    { 0x7C4B3F68, { "ntdll.dll", "NtCreateFile" } },
    { 0x7C4B3F6C, { "ntdll.dll", "RtlInitUnicodeString" } },
    { 0x7C4B3F70, { "ntdll.dll", "NtWriteFile" } },
    { 0x7C4B3F74, { "ntdll.dll", "RtlFreeHeap" } },
    { 0x7C4B3F78, { "ntdll.dll", "NtDelayExecution" } },
    { 0x7C4B3F7C, { "ntdll.dll", "RtlAllocateHeap" } },
};

std::unordered_map<DWORD, std::pair<const char*, const char*>> interfaces = {
    { 0x43AFF050, { "engine.dll", "VEngineClient" } },
    { 0x43AFF01C, { "engine.dll", "EngineTraceClient" } },
    { 0x43AFF04C, { "engine.dll", "VModelInfoClient0" } },
    { 0x43AFF088, { "engine.dll", "VDebugOverlay" } },
    { 0x43AFF094, { "client.dll", "VClient" } },
    { 0x43AFEFDC, { "client.dll", "VClientEntityList" } },
    { 0x43AFF0A8, { "client.dll", "VClientPrediction0" } },
    { 0x43AFF0AC, { "client.dll", "GameMovement0" } },
    { 0x43AFF000, { "materialsystem.dll", "VMaterialSystem" } },
    { 0x43AFF098, { "vguimatsurface.dll", "VGUI_Surface" } },
    { 0x43AFF028, { "vphysics.dll", "VPhysicsSurfaceProps" } },
    { 0x43AFF030, { "vstdlib.dll", "RandomFloat" } },
    { 0x43AFF054, { "vstdlib.dll", "RandomInt" } },
    { 0x43AFF03C, { "vstdlib.dll", "RandomSeed" } },
    { 0x43AFF044, { "vstdlib.dll", "VEngineCvar" } },
    { 0x43AFF048, { "localize.dll", "Localize_" } },
    { 0x43AFF0B0, { "datacache.dll", "MDLCache" } },
};

std::unordered_map<DWORD, std::pair<const char*, const char*>> patterns = {
    { 0x43AFF0F0, { "client.dll", "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" } },
    { 0x43AFF0F4, { "vguimatsurface.dll", "8B 0D ? ? ? ? 56 C6 05" } },
    { 0x43AFF0F8, { "vguimatsurface.dll", "55 8B EC 83 E4 ? 83 EC ? 80 3D" } },
    { 0x43AFF040, { "client.dll", "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE" } },
#ifndef CSGO2018
    { 0x43AFEF3C, { "client.dll", "66 3B 0D ? ? ? ? 72" } },
    { 0x43AFEFA0, { "client.dll", "55 8B EC 51 8B C1 53 56 8B 75" } },
    { 0x43AFEFD8, { "client.dll", "55 8B EC 83 E4 ? 83 EC ? 6A ? 8D 44 24" } },
    { 0x43AFEFC4, { "client.dll", "A1 ? ? ? ? A8 ? 75 ? 0F 57 C0 C7 05 ? ? ? ? 00 00 00 00 F3 0F 7F 05 ? ? ? ? 83 C8 ? C7 05 ? ? ? ? 00 00 00 00 66 0F 6F 05 ? ? ? ? 68 ? ? ? ? A3 ? ? ? ? F3 0F 7F 05 ? ? ? ? C7 05 ? ? ? ? 00 00 00 00 E8 ? ? ? ? 83 C4 ? B8" } },
#else
    { 0x43AFEF3C, { "client.dll", "55 8B EC 81 EC ? ? ? ? 53 8B D9 56 57 8D 8B ? ? ? ? 85 C9 75 04 33 FF EB 2F" } },
    { 0x43AFEFA0, { "client.dll", "55 8B EC 53 8B 5D ? 56 57 8B F9 33 F6 39 77 ? 7E ? 8B 47 ? ? ? ? ? ? FF 50" } },
    { 0x43AFEFD8, { "client.dll", "55 8B EC 83 E4 ? 83 EC ? 6A ? 8D 44 24 ? 89 4C 24" } },
    { 0x43AFEFC4, { "client.dll", "A1 ? ? ? ? A8 01 75 4B" } },
#endif
    { 0x43AFF034, { "client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10" } },
};

std::unordered_map<DWORD, std::pair<const char*, bool>> convars = {
    { 0x43AFEFE4, { "m_yaw", false } },
    { 0x43AFEFEC, { "m_pitch", false } },
    { 0x43AFEFD4, { "sensitivity", false } },
    { 0x43AFEFF0, { "sv_cheats", false } },
    { 0x43AFEFF8, { "sv_footsteps", false } },
    { 0x43AFF008, { "cl_modelfastpath", false } },
    { 0x43AFF02C, { "weapon_recoil_scale", false } },
    { 0x43AFF07C, { "sv_gravity", false } },
    { 0x43AFF09C, { "cl_interpolate", false } },
    { 0x43AFF0B8, { "view_recoil_tracking", false } },
    { 0x43AFF05C, { "sv_maxupdaterate", false } },
    { 0x43AFF060, { "cl_interp", false } },
    { 0x43AFF064, { "sv_minupdaterate", false } },
    { 0x43AFF068, { "cl_interp_ratio", false } },
    { 0x43AFF070, { "sv_client_max_interp_ratio", false } },
    { 0x43AFF074, { "cl_updaterate", false } },
    { 0x43AFF078, { "sv_client_min_interp_ratio", false } },
    { 0x43AFF080, { "molotov_throw_detonate_time", false } },
    { 0x43AFF084, { "weapon_molotov_maxdetonateslope", false } },
    { 0x43AFF00C, { "name", false } },
    { 0x43AFEFE0, { "r_3dsky", false } },
    { 0x43AFF010, { "r_drawskybox", false } },
    { 0x43AFF014, { "gl_clear", false } },
    { 0x43AFF058, { "cl_fullupdate", true } },
};

std::vector<DWORD> xor_patches = {
    0x34E1EAAD, 0x34E1EB16, 0x34E1EB86,
    0x34E1FE4A, 0x34E20169, 0x34E208A5, 0x34E20995,
    0x34E20A8A, 0x34E20BC6, 0x34E20D55, 0x34E20E2F,
    0x34E1DFC9, 0x34E1E2DA, 0x34E1E32A, 0x34E1E38E,
    0x34E1F14E, 0x34E1F190, 0x34E1F1D0,
    0x34E239C7, 0x34E1F584,
    0x34E1F26A, 0x34E1F2B5, 0x34E1F2F8,
    0x34E1F002, 0x34E1F045, 0x34E1F088,
    0x34E25A13,
    0x34E2AD02, 0x34E2AC29,
    0x34E30DE4, 0x34E2B4BB,
    0x34E31D7B, 0x34E30FE4,
    0x34E37D73, 0x34E3791E,
    0x34E38185, 0x34E3828B, 0x34E382D3,
    0x34E388A4, 0x34E389AF, 0x34E38A52,
    0x34E38FB2, 0x34E39396,
    0x34E372C5, 0x34E37074,
    0x34E36CC5, 0x34E36C45, 0x34E36BC5, 0x34E36B45,
    0x34E35983, 0x34E357C5, 0x34E34746,
    0x34E37EEF, 0x34E3837E, 0x34E38C48, 0x34E394BF, 0x34E39C5B,
    0x34E3783F, 0x34E3774E, 0x34E36A6F, 0x34E3640E,
    0x34E35028, 0x34E350C1, 0x34E34D58,
    0x34E34C1A, 0x34E34C4C, 0x34E343AA, 0x34E34464,
    0x34E2B347, 0x34E2B2EA, 0x34E2B473,
};

// ===================== Implementation =====================

namespace Aimware {

bool InitializeMemoryDumps() {
    LOG_INFO("Initializing memory dumps...");

    auto& mem = MemoryManager::Instance();

    if (!mem.AllocateFixed(0x7C4A0000, sizeof(b7C4A0000), "CRT Helper", b7C4A0000)) {
        LOG_ERROR("Failed to allocate b7C4A0000");
        return false;
    }

    if (!mem.AllocateFixed(0x76ED0000, sizeof(b76ED0000), "String Resources", b76ED0000)) {
        LOG_ERROR("Failed to allocate b76ED0000");
        return false;
    }

    if (!mem.AllocateFixed(0x43AF0000, sizeof(b43AF0000), "Data Section", b43AF0000)) {
        LOG_ERROR("Failed to allocate b43AF0000");
        return false;
    }

    if (!mem.AllocateFixed(0x34E10000, sizeof(b34E10000), "Code Section", b34E10000)) {
        LOG_ERROR("Failed to allocate b34E10000");
        return false;
    }

    LOG_SUCCESS("All dumps allocated");
    return true;
}

void FixImports() {
    LOG_INFO("Fixing imports (%zu entries)...", imports.size());

    int fixed = 0, failed = 0;
    for (auto& [addr, modFunc] : imports) {
        HMODULE mod = LoadLibraryA(modFunc.first);
        if (!mod) {
            mod = GetModuleHandleA(modFunc.first);
            if (!mod) {
                LOG_WARN("Module %s not found", modFunc.first);
                failed++;
                continue;
            }
        }

        FARPROC proc = GetProcAddress(mod, modFunc.second);
        if (!proc) {
            LOG_WARN("Import %s::%s not found", modFunc.first, modFunc.second);
            failed++;
            continue;
        }

        *(FARPROC*)addr = proc;
        fixed++;
    }

    LOG_SUCCESS("Imports fixed: %d ok, %d failed", fixed, failed);
}

void FixAddresses() {
    LOG_INFO("Fixing addresses (%zu patterns)...", patterns.size());

    for (auto& [addr, modPat] : patterns) {
        uintptr_t found = PatternScanner::FindOrZero(modPat.first, modPat.second);
        if (!found) {
            LOG_WARN("Pattern not found: %s [%s]", modPat.first, modPat.second);
            continue;
        }
        *(uintptr_t*)addr = found;
        LOG_INFO("Pattern %s -> 0x%08X at 0x%08X", modPat.first, found, addr);
    }

    // Additional fixes
    auto findAndSet = [](DWORD targetAddr, const char* mod, const char* pat, int offset = 0) {
        uintptr_t found = PatternScanner::FindOrZero(mod, pat);
        if (found) {
            DWORD value = *(DWORD*)(found + offset);
            // If pattern points to instruction with immediate, extract
            // For simplicity, handle common case where we need to read dword at +1 or +2
            if (targetAddr) {
                // Special handling per address
            }
        }
    };

    // pHud
    uintptr_t pHudSig = PatternScanner::FindOrZero("client.dll", "B9 ? ? ? ? 56 68 ? ? ? ? 89 45");
    if (pHudSig) {
        *(PDWORD)0x43AFEFA4 = *(PDWORD)(pHudSig + 1);
        LOG_INFO("pHud fixed: 0x%08X", *(PDWORD)0x43AFEFA4);
    }

    // PredictionRandomSeed
    uintptr_t predSeed = PatternScanner::FindOrZero("client.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04");
    if (predSeed) {
        *(PDWORD)0x43AFEE5C = *(DWORD*)(predSeed + 2);
        LOG_INFO("PredictionRandomSeed fixed");
    }

    // SmokeCount
    uintptr_t smokeCount = PatternScanner::FindOrZero("client.dll", "A3 ? ? ? ? 57 8B CB");
    if (smokeCount) {
        *(PDWORD)0x43AFF0B4 = *(DWORD*)(smokeCount + 1);
        LOG_INFO("SmokeCount fixed: 0x%08X", *(PDWORD)0x43AFF0B4);
    }

    // Name spam
    uintptr_t nameSpam = PatternScanner::FindOrZero("engine.dll", "38 05 ? ? ? ? 75 ? 8B CE C6 05 ? ? ? ? ? E8 ? ? ? ? C6 05 ? ? ? ? 00");
    if (nameSpam) {
        *(PDWORD)0x43AFF004 = *(DWORD*)(nameSpam + 2);
    }

    // CInput offsets
    DWORD inputBase = *(PDWORD)0x43AFF06C;
    if (inputBase) {
        *(PDWORD)0x43AFEF4C = inputBase + 0xA8;
        *(PDWORD)0x43AFEF54 = inputBase + 0xA5;
    }

    // Trace filters
    uintptr_t traceSimple = PatternScanner::FindOrZero("client.dll", "C7 45 ? ? ? ? ? C7 45 ? 00 00 00 00 FF 50 ? A1");
    if (traceSimple) {
        *(PDWORD)0x43AFF0FC = *(DWORD*)(traceSimple + 3);
    }

    uintptr_t traceSkipTwo = PatternScanner::FindOrZero("client.dll", "C7 44 24 ? ? ? ? ? FF 90 ? ? ? ? 8D 44 24 ? 50 8D 44 24 ? 50 68 ? ? ? ? 8B 55");
    if (traceSkipTwo) {
        *(PDWORD)0x43AFF100 = *(DWORD*)(traceSkipTwo + 4);
    }

    LOG_SUCCESS("Address fixing complete");
}

void FixConvars() {
    LOG_INFO("Fixing convars (%zu entries)...", convars.size());

    auto& state = GlobalState::Instance();
    if (!state.cvars) {
        state.cvars = GetInterface<ICvar>("vstdlib.dll", "VEngineCvar");
    }

    if (!state.cvars) {
        LOG_ERROR("ICvar interface not found");
        return;
    }

    int fixed = 0;
    for (auto& [addr, nameIsCmd] : convars) {
        void* cvar_ptr = nameIsCmd.second ?
            (void*)state.cvars->FindCommand(nameIsCmd.first) :
            (void*)state.cvars->FindVar(nameIsCmd.first);

        if (!cvar_ptr) {
            LOG_WARN("Cvar %s not found", nameIsCmd.first);
            continue;
        }

        *(void**)addr = cvar_ptr;
        fixed++;
    }

    LOG_SUCCESS("Convars fixed: %d/%zu", fixed, convars.size());
}

bool InitializeInterfaces() {
    LOG_INFO("Initializing interfaces (%zu entries)...", interfaces.size());

    auto& state = GlobalState::Instance();
    int fixed = 0;

    for (auto& [addr, modName] : interfaces) {
        void* iface = nullptr;

        if (strstr(modName.second, "Random")) {
            HMODULE mod = GetModuleHandleA(modName.first);
            if (mod) iface = GetProcAddress(mod, modName.second);
        } else {
            iface = GetInterface<void>(modName.first, modName.second);
        }

        if (!iface) {
            LOG_WARN("Interface %s::%s not found", modName.first, modName.second);
            continue;
        }

        *(void**)addr = iface;
        fixed++;
    }

    // Resolve additional interfaces
    state.client = *(IBaseClientDLL**)(0x43AFF094);
    if (!state.client) {
        LOG_ERROR("Client interface null");
        return false;
    }

    // ClientMode - from client vtable[10] + 5
    try {
        state.client_mode = **(void***)((*(DWORD**)state.client)[10] + 0x5);
        LOG_INFO("ClientMode: 0x%p", state.client_mode);
    } catch (...) {
        LOG_ERROR("Failed to get ClientMode");
    }

    state.prediction = *(void**)(0x43AFF0A8);
    state.surface = *(void**)(0x43AFF098);
    state.trace = *(void**)(0x43AFF01C);

    // GlobalVarsBase
    try {
#ifndef CSGO2018
        *(PDWORD)0x43AFF020 = **(DWORD**)((*(DWORD**)(state.client))[0] + 0x53);
#else
        *(PDWORD)0x43AFF020 = **(DWORD**)((*(DWORD**)(state.client))[0] + 0x1B);
#endif
        LOG_INFO("GlobalVarsBase fixed");
    } catch (...) {
        LOG_ERROR("Failed to fix GlobalVarsBase");
    }

    // CInput
    try {
        *(PDWORD)0x43AFF06C = *reinterpret_cast<DWORD*>((*reinterpret_cast<uintptr_t**>(state.client))[15] + 0x1);
        LOG_INFO("CInput fixed: 0x%08X", *(PDWORD)0x43AFF06C);
    } catch (...) {
        LOG_ERROR("Failed to fix CInput");
    }

    // MoveHelper
    uintptr_t moveHelperSig = PatternScanner::FindOrZero("client.dll", "8B 0D ? ? ? ? 8B 46 08 68");
    if (moveHelperSig) {
        *(PDWORD)0x43AFEFE8 = **(DWORD**)(moveHelperSig + 0x2);
        LOG_INFO("MoveHelper fixed");
    }

    // ItemSystem
    uintptr_t itemSystemSig = PatternScanner::FindOrZero("client.dll", "A1 ? ? ? ? 85 C0 75 ? A1 ? ? ? ? 56 68");
    if (itemSystemSig) {
        auto getItemSystem = reinterpret_cast<CCStrike15ItemSystem*(*)()>(itemSystemSig);
        *(PDWORD)0x43AFEFD0 = (DWORD)getItemSystem();
        LOG_INFO("ItemSystem fixed: 0x%08X", *(PDWORD)0x43AFEFD0);
    }

    // Local interfaces for hooking
    state.engine_vgui = GetInterface<void>("engine.dll", "VEngineVGui0");
    if (state.engine_vgui) {
        state.engine_vgui_hook = std::make_unique<VMTHook>((void**)state.engine_vgui);
        state.engine_vgui_hook->Initialize((void**)state.engine_vgui);
    }

    void* studio_render = GetInterface<void>("studiorender.dll", "VStudioRender");
    if (studio_render) {
        state.studio_render = studio_render;
        state.studio_render_hook = std::make_unique<VMTHook>((void**)studio_render);
        state.studio_render_hook->Initialize((void**)studio_render);
    }

    state.client_hook = std::make_unique<VMTHook>((void**)state.client);
    state.client_hook->Initialize((void**)state.client);

    if (state.client_mode) {
        state.client_mode_hook = std::make_unique<VMTHook>((void**)state.client_mode);
        state.client_mode_hook->Initialize((void**)state.client_mode);
    }

    if (state.prediction) {
        state.prediction_hook = std::make_unique<VMTHook>((void**)state.prediction);
        state.prediction_hook->Initialize((void**)state.prediction);
    }

    if (state.surface) {
        state.surface_hook = std::make_unique<VMTHook>((void**)state.surface);
        state.surface_hook->Initialize((void**)state.surface);
    }

    if (state.trace) {
        state.trace_hook = std::make_unique<VMTHook>((void**)state.trace);
        state.trace_hook->Initialize((void**)state.trace);
    }

    LOG_SUCCESS("Interfaces initialized: %d/%zu", fixed, interfaces.size());
    return fixed > 0;
}

bool InitializeNetvars() {
    LOG_INFO("Initializing netvars...");

    auto& state = GlobalState::Instance();
    if (!state.client) {
        LOG_ERROR("Client null for netvar init");
        return false;
    }

    bool result = state.netvars.Initialize(state.client);
    if (!result) {
        LOG_ERROR("NetvarManager init failed");
        return false;
    }

    LOG_SUCCESS("Netvars initialized: %zu tables", state.netvars.tables.size());
    return true;
}

void FixPostOEP() {
    LOG_INFO("Fixing post-OEP crap...");

    auto& state = GlobalState::Instance();
    auto& mem = MemoryManager::Instance();

    // Fix render
    AwRender* render = *(AwRender**)(0x43B01224);
    if (render) {
        LOG_INFO("Render original res: %dx%d", render->Width, render->Height);
        render->DidCreateFont = false;
        render->Width = 0;
        render->Height = 0;
    }

    // Fix XOR patches - set to JNZ (0x75)
    LOG_INFO("Patching XOR checks (%zu)...", xor_patches.size());
    for (auto addr : xor_patches) {
        mem.PatchByte(addr, 0x75);
    }

    // Find window
    HWND window = nullptr;
    int attempts = 0;
    while (!(window = FindWindowA("Valve001", nullptr)) && attempts < 100) {
        Sleep(100);
        attempts++;
    }

    if (window) {
        state.window = window;
        LOG_SUCCESS("Window found: 0x%p", window);
    } else {
        LOG_WARN("Window Valve001 not found, using foreground");
        state.window = GetForegroundWindow();
    }

    // Reset skinchanger struct
    AwSkinChangerData* skinCtx = *(AwSkinChangerData**)(0x43AF7700);
    if (skinCtx) {
        memset(skinCtx, 0, 216);
        state.skinchanger_ctx = skinCtx;
    }

    // Fix netvar proxies
    state.netvars.GetProp("CCSPlayer", "m_angEyeAngles[1]", (RecvProp**)0x43B01318);
    state.netvars.GetProp("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", (RecvProp**)0x43B01324);

    *(PDWORD)0x43AFE0C4 = (DWORD)state.netvars.GetClass("CBaseWeaponWorldModel");
    *(PDWORD)0x43AFE0E0 = (DWORD)state.netvars.GetClass("CBaseViewModel");

    // Client.dll info for pattern scanner inside cheat
    MODULEINFO info;
    if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("client.dll"), &info, sizeof(info))) {
        *(PDWORD)0x43AFF038 = (DWORD)info.lpBaseOfDll + 0x1000;
        *(PDWORD)0x43AFF024 = info.SizeOfImage;
        LOG_INFO("Client.dll base: 0x%p size: 0x%X", info.lpBaseOfDll, info.SizeOfImage);
    }

    // Allocate lag records
    void* records = mem.AllocateLagRecords();
    if (!records) {
        LOG_ERROR("Failed to allocate lag records");
    }

    *(PDWORD)0x43AFE638 = 0; // UnlockCursor flag

    // Profile context
    struct profile_t {
        int xor_key = 0;
        int pad;
        wchar_t config_path[260];
        int pad2[64];
    };

    profile_t* profile_ = (profile_t*)mem.AllocateDynamic(sizeof(profile_t), "Profile");
    memset(profile_, 0, sizeof(profile_t));
    profile_->xor_key = 0;
    memcpy(profile_->config_path, L"\\??\\C:\\aimware\\", sizeof(L"\\??\\C:\\aimware\\"));
    *(PDWORD)0x43AFF218 = (DWORD)profile_;

    CreateDirectoryA("C:\\aimware\\", 0);
    ConfigSystem::Instance().Initialize(L"C:\\aimware\\");

    // NOP out some checks in skinchanger
    mem.NopRange(0x34E1DA71, 0x34E1DA82);
    mem.NopRange(0x34E1DA85, 0x34E1DA8A);
    mem.NopRange(0x34E1DA8F, 0x34E1DA94);
    mem.NopRange(0x34E1DA9A, 0x34E1DA9D);
    mem.NopRange(0x34E1DAA2, 0x34E1DAA5);
    mem.NopRange(0x34E1DAA8, 0x34E1DAAD);

#ifdef CSGO2018
    // 2018 fixes - offsets
    *(PDWORD)0x34E1D948 += 0x24;
    *(PDWORD)0x34E1D954 += 0x24;
    *(PDWORD)0x34E1D960 += 0x24;
    *(PDWORD)0x34E1D989 += 0x24;
    *(PDWORD)0x34E1DA1D += 0x24;
    *(PDWORD)0x34E1D9A2 += 0x24;
    *(PDWORD)0x34E1DB2F += 0x24;
    *(PDWORD)0x34E1DB52 += 0x24;
    *(PDWORD)0x34E1DBEA += 0x24;
    *(PDWORD)0x34E1DC3A += 0x24;
    *(PDWORD)0x34E1DC1B += 0x24;
    *(PDWORD)0x34E31F0B += 0x24;
    *(PDWORD)0x34E31F15 += 0x24;

    *(PDWORD)0x34E1C021 = 469 * 4;
    *(PDWORD)0x34E1C015 = 471 * 4;
    *(PDWORD)0x34E1BFD6 = 439 * 4;

    mem.PatchBytes(0x34E1C038, { 0x89, 0xF9, 0x90, 0x90 });
    *(PSHORT)0x34E1C046 = 0x00C8;
    *(PSHORT)0x34E1C052 = 0x00F0;
    *(PSHORT)0x34E1C05E = 0x00F8;
    *(PSHORT)0x34E1C06A = 0x00EC;
    *(PSHORT)0x34E1C076 = 0x0104;
    *(PSHORT)0x34E1C082 = 0x0108;
    *(PSHORT)0x34E1C08E = 0x00F4;
    *(PSHORT)0x34E1C09A = 0x0000;

    mem.NopRange(0x34E318C6, 0x34E318D7);
    mem.NopRange(0x34E318DA, 0x34E318DF);
    mem.NopRange(0x34E318E4, 0x34E31918);

    // Netvar fixes for 2018
    *(PDWORD)0x43AFEF34 = 0x29BC;
    *(PDWORD)0x43AFEFC8 = 0xA310;
    *(PDWORD)0x43AFEFB8 = 0x32B0;
    *(PDWORD)0x43AFEED8 = 0x3360;
    *(PDWORD)0x43AFEEE0 = state.netvars.GetOffset("CCSPlayer", "deadflag") + 4;
    *(PDWORD)0x43AFEEE4 = state.netvars.GetOffset("CCSPlayer", "m_nTickBase");
    *(PDWORD)0x43AFEEA0 = state.netvars.GetOffset("CCSPlayer", "m_flNextAttack");
    *(PDWORD)0x43AFEEB4 = state.netvars.GetOffset("CCSPlayer", "m_flPoseParameter");
    *(PDWORD)0x43AFEEBC = state.netvars.GetOffset("CCSPlayer", "m_bClientSideAnimation");
    *(PDWORD)0x43AFEF70 = state.netvars.GetOffset("CCSPlayer", "m_bHasHelmet");
    *(PDWORD)0x43AFEF74 = state.netvars.GetOffset("CCSPlayer", "m_bHasDefuser");
    *(PDWORD)0x43AFEF78 = state.netvars.GetOffset("CCSPlayer", "m_iAccount");
    *(PDWORD)0x43AFEF7C = state.netvars.GetOffset("CCSPlayer", "m_bIsDefusing");
    *(PDWORD)0x43AFEF80 = state.netvars.GetOffset("CCSPlayer", "m_flLowerBodyYawTarget");
    *(PDWORD)0x43AFEF84 = state.netvars.GetOffset("CCSPlayer", "m_iShotsFired");
    *(PDWORD)0x43AFEF88 = state.netvars.GetOffset("CCSPlayer", "m_bGunGameImmunity");
    *(PDWORD)0x43AFEE9C = state.netvars.GetOffset("CCSPlayer", "m_aimPunchAngle");
    *(PDWORD)0x43AFEF6C = state.netvars.GetOffset("CBasePlayer", "m_ArmorValue");
    *(PDWORD)0x43AFEEA8 = state.netvars.GetOffset("CBaseCombatCharacter", "m_hActiveWeapon");
    *(PDWORD)0x43AFEEAC = state.netvars.GetOffset("CBaseCombatCharacter", "m_hMyWearables");
    *(PDWORD)0x43AFEEF4 = 0x31D8;
    *(PDWORD)0x43AFEEF8 = state.netvars.GetOffset("CBaseCombatWeapon", "m_iItemDefinitionIndex");
    *(PDWORD)0x43AFEEFC = state.netvars.GetOffset("CBaseCombatWeapon", "m_iClip1");
    *(PDWORD)0x43AFEF38 = state.netvars.GetOffset("CBaseCombatWeapon", "m_iClip2");
    *(PDWORD)0x43AFEF40 = state.netvars.GetOffset("CBaseCombatWeapon", "m_iAccountID");
    *(PDWORD)0x43AFEF20 = state.netvars.GetOffset("CBaseCombatWeapon", "m_iViewModelIndex");
    *(PDWORD)0x43AFEF24 = state.netvars.GetOffset("CBaseCombatWeapon", "m_iWorldModelIndex");
    *(PDWORD)0x43AFEF30 = state.netvars.GetOffset("CBaseCombatWeapon", "m_iPrimaryReserveAmmoCount");
    *(PDWORD)0x43AFEFB4 = state.netvars.GetOffset("CBaseCombatWeapon", "m_flPostponeFireReadyTime");
    *(PDWORD)0x43AFEF00 = state.netvars.GetOffset("CBaseAttributableItem", "m_nFallbackStatTrak");
    *(PDWORD)0x43AFEF04 = state.netvars.GetOffset("CBaseAttributableItem", "m_nFallbackPaintKit");
    *(PDWORD)0x43AFEF08 = state.netvars.GetOffset("CBaseAttributableItem", "m_OriginalOwnerXuidLow");
    *(PDWORD)0x43AFEF0C = state.netvars.GetOffset("CBaseAttributableItem", "m_bInitialized");
    *(PDWORD)0x43AFEF10 = state.netvars.GetOffset("CBaseAttributableItem", "m_szCustomName");
    *(PDWORD)0x43AFEF14 = state.netvars.GetOffset("CBaseAttributableItem", "m_iItemIDLow");
    *(PDWORD)0x43AFEF28 = state.netvars.GetOffset("CBaseAttributableItem", "m_nFallbackSeed");
    *(PDWORD)0x43AFEF2C = state.netvars.GetOffset("CBaseAttributableItem", "m_flFallbackWear");

    uintptr_t pHud2018 = PatternScanner::FindOrZero("client.dll", "B9 ? ? ? ? 0F 94 C0 0F B6 C0 50 68");
    if (pHud2018) *(PDWORD)0x43AFEFA4 = *(PDWORD)(pHud2018 + 1);

    uintptr_t predSeed2018 = PatternScanner::FindOrZero("client.dll", "C7 05 ? ? ? ? ? ? ? ? EB ? 8B 47");
    if (predSeed2018) *(PDWORD)0x43AFEE5C = *(DWORD*)(predSeed2018 + 2);
#endif

    // Call netvar & skins init
    LOG_INFO("Calling netvar init at 0x34E1D890");
    try {
        ((void(*)())(0x34E1D890))();
    } catch (...) {
        LOG_ERROR("Exception in netvar init");
    }

    LOG_INFO("Refreshing config list at 0x34E2B410");
    try {
        ((void(*)())(0x34E2B410))();
    } catch (...) {
        LOG_ERROR("Exception in config list refresh");
    }

    // Reset config name
    memset((void*)0x43AF8B2C, 0, 32);

    LOG_SUCCESS("Post-OEP fixes done");
}

void HookNetvar(const char* table, const char* var, uintptr_t original_addr, uintptr_t hook_fn) {
    auto& state = GlobalState::Instance();
    RecvProp* prop = nullptr;
    int offset = state.netvars.GetProp(table, var, &prop);

    if (!prop) {
        LOG_WARN("Netvar %s::%s not found", table, var);
        return;
    }

    state.hooked_netvars.push_back({ (uintptr_t)prop->proxy, (uintptr_t)prop });

    if (original_addr) {
        *(void**)original_addr = (void*)prop->proxy;
    }

    prop->proxy = (RecvVarProxyFn)hook_fn;
    LOG_INFO("Hooked netvar %s::%s offset 0x%X -> 0x%08X", table, var, offset, hook_fn);
}

void UnhookNetvars() {
    auto& state = GlobalState::Instance();
    for (auto& [original, propAddr] : state.hooked_netvars) {
        ((RecvProp*)propAddr)->proxy = (RecvVarProxyFn)original;
    }
    state.hooked_netvars.clear();
    LOG_INFO("Netvars unhooked");
}

void __cdecl hkPanic() {
    LOG_WARN("Panic called - unhooking all");

    auto& state = GlobalState::Instance();
    state.panic = true;

    if (state.engine_vgui_hook) state.engine_vgui_hook->Unhook();
    if (state.client_hook) state.client_hook->Unhook();
    if (state.client_mode_hook) state.client_mode_hook->Unhook();
    if (state.prediction_hook) state.prediction_hook->Unhook();
    if (state.surface_hook) state.surface_hook->Unhook();
    if (state.trace_hook) state.trace_hook->Unhook();
    if (state.studio_render_hook) state.studio_render_hook->Unhook();

    UnhookNetvars();

    if (state.skinchanger_ctx) {
        ((RecvProp*)state.skinchanger_ctx->sequence_prop)->proxy = (RecvVarProxyFn)state.skinchanger_ctx->sequence_proxy;
    }

    if (state.window && state.orig_wndproc) {
        SetWindowLongPtr(state.window, GWL_WNDPROC, (LONG)state.orig_wndproc);
    }

    LOG_SUCCESS("Panic unhook complete");
}

void __stdcall hkGenConfigPath(const char* name, wchar_t* out) {
    // Use improved config system but keep binary compatibility
    Aimware2016Decompiled::ConfigUIEngine::hkGenConfigPath(name, out);
}

bool InitializeHooks() {
    LOG_INFO("Initializing hooks...");

    auto& state = GlobalState::Instance();

    // Detour config path generator to make filenames readable
    DetourFunction((PBYTE)0x34E34E90, (PBYTE)hkGenConfigPath);
    DetourFunction((PBYTE)0x34E25530, (PBYTE)hkPanic);

    if (state.window) {
        state.orig_wndproc = (WNDPROC)SetWindowLongPtr(state.window, GWL_WNDPROC, (LONG_PTR)((void*)0x34E33D60));
        *(WNDPROC*)(0x43AFF104) = state.orig_wndproc;
        *(HWND*)(0x43AFF214) = state.window;
        LOG_INFO("WndProc hooked: original 0x%p", state.orig_wndproc);
    }

#ifdef USE_DECOMPILED_ENGINE
    LOG_INFO("Using DECOMPILED engine hooks");

    if (state.engine_vgui_hook) {
        *(PDWORD)0x43AFE63C = state.engine_vgui_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkEngineVGUIPaint, 14);
    }
    if (state.client_hook) {
        *(PDWORD)0x43AFE630 = state.client_hook->HookFunction(0x34E26330, 36);
        state.client_hook->HookFunction(0x34E268C0, 23);
    }
    if (state.client_mode_hook) {
        *(PDWORD)0x43AFE178 = state.client_mode_hook->HookFunction((DWORD)&Aimware2016Decompiled::AimbotEngine::hkCreateMove, 24);
    }
    if (state.prediction_hook) {
        *(PDWORD)0x43AFEE54 = state.prediction_hook->HookFunction(0x34E314B0, 19);
        *(PDWORD)0x43AFE644 = state.prediction_hook->HookFunction(0x34E26880, 20);
        state.prediction_hook->HookFunction(0x34E26500, 14);
    }
    if (state.surface_hook) {
        *(PDWORD)0x43AFE634 = state.surface_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkLockCursor, 67);
    }
    if (state.studio_render_hook) {
        *(PDWORD)0x43AFE17C = state.studio_render_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkDrawModel, 29);
    }

    *(PDWORD)0x43AFE0E8 = (DWORD)DetourFunction((PBYTE)PatternScanner::FindOrZero("client.dll", "55 8B EC 83 EC ? 56 8B F1 57 89 75 ? E8 ? ? ? ? 8B CE E8"), (PBYTE)&Aimware2016Decompiled::ResolverEngine::hkOnRenderStart);

    HookNetvar("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", 0x43B01328, 0x34E24BC0);
    HookNetvar("CCSPlayer", "m_angEyeAngles[0]", 0, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnPitchProxy);
    HookNetvar("CCSPlayer", "m_angEyeAngles[1]", 0, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnYawProxy);
    HookNetvar("CCSPlayer", "m_flThirdpersonRecoil", 0x43B01304, 0x34E24FC0);
    HookNetvar("CCSPlayer", "m_flLowerBodyYawTarget", 0x43B01334, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnLBYProxy);
    HookNetvar("CBasePlayer", "m_fFlags", 0x43B01340, 0x34E24F40);
    HookNetvar("CCSPlayer", "m_flFlashDuration", 0x43B0134C, 0x34E24B90);

#else
    LOG_INFO("Using ORIGINAL binary hooks");

    if (state.engine_vgui_hook) *(PDWORD)0x43AFE63C = state.engine_vgui_hook->HookFunction(0x34E26660, 14);
    if (state.client_hook) {
        *(PDWORD)0x43AFE630 = state.client_hook->HookFunction(0x34E26330, 36);
        state.client_hook->HookFunction(0x34E268C0, 23);
    }
    if (state.client_mode_hook) *(PDWORD)0x43AFE178 = state.client_mode_hook->HookFunction(0x34E258A0, 24);
    if (state.prediction_hook) {
        *(PDWORD)0x43AFEE54 = state.prediction_hook->HookFunction(0x34E314B0, 19);
        *(PDWORD)0x43AFE644 = state.prediction_hook->HookFunction(0x34E26880, 20);
        state.prediction_hook->HookFunction(0x34E26500, 14);
    }
    if (state.surface_hook) *(PDWORD)0x43AFE634 = state.surface_hook->HookFunction(0x34E26620, 67);
    if (state.studio_render_hook) *(PDWORD)0x43AFE17C = state.studio_render_hook->HookFunction(0x34E25960, 29);

    *(PDWORD)0x43AFE0E8 = (DWORD)DetourFunction((PBYTE)PatternScanner::FindOrZero("client.dll", "55 8B EC 83 EC ? 56 8B F1 57 89 75 ? E8 ? ? ? ? 8B CE E8"), (PBYTE)0x34E25040);

    HookNetvar("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", 0x43B01328, 0x34E24BC0);
    HookNetvar("CCSPlayer", "m_angEyeAngles[0]", 0, 0x34E24BF0);
    HookNetvar("CCSPlayer", "m_angEyeAngles[1]", 0, 0x34E24D20);
    HookNetvar("CCSPlayer", "m_flThirdpersonRecoil", 0x43B01304, 0x34E24FC0);
    HookNetvar("CCSPlayer", "m_flLowerBodyYawTarget", 0x43B01334, 0x34E24EC0);
    HookNetvar("CBasePlayer", "m_fFlags", 0x43B01340, 0x34E24F40);
    HookNetvar("CCSPlayer", "m_flFlashDuration", 0x43B0134C, 0x34E24B90);
#endif

    LOG_SUCCESS("Hooks initialized");
    return true;
}

void Shutdown() {
    LOG_WARN("Shutting down Aimware...");

    auto& state = GlobalState::Instance();
    state.Reset();
    MemoryManager::Instance().FreeAll();

    LOG_INFO("Shutdown complete");
}

} // namespace Aimware

// ===================== Main Thread =====================

DWORD WINAPI install_thread(PVOID) {
    Aimware::Logger::Instance().Initialize(true, false);
    LOG_INFO("=== Aimware 2016 Loader Started ===");

    // Wait for serverbrowser.dll - indicates game fully loaded
    LOG_INFO("Waiting for serverbrowser.dll...");
    while (!GetModuleHandleA("serverbrowser.dll")) {
        Sleep(100);
    }
    Sleep(500); // Extra wait for all modules

    LOG_INFO("Game modules loaded, initializing...");

    if (!Aimware::InitializeMemoryDumps()) {
        LOG_ERROR("Failed to initialize memory dumps");
        return 1;
    }

    // Initialize global pointers from fixed memory
    auto& state = GlobalState::Instance();
    state.render = *(AwRender**)(0x43B01224);
    state.global_ctx = *(AwGlobals**)(0x43AF7704);
    state.skinchanger_ctx = *(AwSkinChangerData**)(0x43AF7700);

    LOG_INFO("Fixing imports...");
    Aimware::FixImports();

    LOG_INFO("Initializing interfaces...");
    if (!Aimware::InitializeInterfaces()) {
        LOG_ERROR("Interface initialization failed");
        // Continue anyway
    }

    LOG_INFO("Initializing netvars...");
    if (!Aimware::InitializeNetvars()) {
        LOG_ERROR("Netvar initialization failed");
    }

    LOG_INFO("Fixing addresses...");
    Aimware::FixAddresses();

    LOG_INFO("Fixing convars...");
    Aimware::FixConvars();

    LOG_INFO("Fixing post-OEP...");
    Aimware::FixPostOEP();

    LOG_INFO("Initializing decompiled engine...");
    Aimware2016Decompiled::InitializeDecompiledEngine();

    LOG_INFO("Initializing hooks...");
    if (!Aimware::InitializeHooks()) {
        LOG_ERROR("Hook initialization failed");
        return 1;
    }

    state.initialized = true;
    LOG_SUCCESS("=== Aimware 2016 Loaded Successfully ===");
    LOG_INFO("Press INSERT to open menu (if available)");
    LOG_INFO("Fixed memory regions: 0x34E10000, 0x43AF0000, 0x76ED0000, 0x7C4A0000");
    LOG_INFO("Decompiled engine: %s", 
#ifdef USE_DECOMPILED_ENGINE
        "ENABLED"
#else
        "DISABLED (binary)"
#endif
    );

    return 0;
}

BOOL WINAPI DllMain(void* hinst, int reason, void* reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls((HMODULE)hinst);
        CreateThread(0, 0, install_thread, 0, 0, 0);
    } else if (reason == DLL_PROCESS_DETACH) {
        if (GlobalState::Instance().initialized && !GlobalState::Instance().panic) {
            Aimware::Shutdown();
        }
    }
    return TRUE;
}
