#include "aw.h"
#include "detours.h"

#define CSGO2016

using namespace Aimware;
using namespace Aimware::Utils;

bool call_in_bounds(void* addr) {
    uintptr_t u = (uintptr_t)addr;
    return u > 0x34E10000 && u < (0x34E10000 + sizeof(b34E10000));
}

// Imports
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
    { 0x43AFEF3C, { "client.dll", "66 3B 0D ? ? ? ? 72" } },
    { 0x43AFEFA0, { "client.dll", "55 8B EC 51 8B C1 53 56 8B 75" } },
    { 0x43AFEFD8, { "client.dll", "55 8B EC 83 E4 ? 83 EC ? 6A ? 8D 44 24" } },
    { 0x43AFEFC4, { "client.dll", "A1 ? ? ? ? A8 ? 75 ? 0F 57 C0 C7 05 ? ? ? ? 00 00 00 00 F3 0F 7F 05 ? ? ? ? 83 C8 ? C7 05 ? ? ? ? 00 00 00 00 66 0F 6F 05 ? ? ? ? 68 ? ? ? ? A3 ? ? ? ? F3 0F 7F 05 ? ? ? ? C7 05 ? ? ? ? 00 00 00 00 E8 ? ? ? ? 83 C4 ? B8" } },
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
    0x34E239C7, 0x34E1F584, 0x34E1F26A, 0x34E1F2B5, 0x34E1F2F8,
    0x34E1F002, 0x34E1F045, 0x34E1F088,
    0x34E25A13, 0x34E2AD02, 0x34E2AC29, 0x34E30DE4, 0x34E2B4BB,
    0x34E31D7B, 0x34E30FE4, 0x34E37D73, 0x34E3791E,
    0x34E38185, 0x34E3828B, 0x34E382D3, 0x34E388A4, 0x34E389AF, 0x34E38A52,
    0x34E38FB2, 0x34E39396, 0x34E372C5, 0x34E37074,
    0x34E36CC5, 0x34E36C45, 0x34E36BC5, 0x34E36B45,
    0x34E35983, 0x34E357C5, 0x34E34746, 0x34E37EEF, 0x34E3837E, 0x34E38C48,
    0x34E394BF, 0x34E39C5B, 0x34E3783F, 0x34E3774E, 0x34E36A6F, 0x34E3640E,
    0x34E35028, 0x34E350C1, 0x34E34D58, 0x34E34C1A, 0x34E34C4C, 0x34E343AA, 0x34E34464,
    0x34E2B347, 0x34E2B2EA, 0x34E2B473,
};

namespace Aimware {

// ===================== Mapper =====================
bool InitializeMemoryDumps() {
    LOG_INFO("=== Mapper: Initializing ===");
    
    auto& mapper = Mapping::Mapper::Instance();
    mapper.AddDefaultRegions();
    
    // Map with data
    const uint8_t* datas[] = {
        b43AF0000,
        b34E10000,
        b7C4A0000,
        b76ED0000
    };
    
    // Need to match order: DATA, CODE, CRT, STRING
    // kAimwareRegions order is DATA, CODE, CRT, STRING - matches datas array
    
    for (size_t i = 0; i < mapper.GetRegions().size(); ++i) {
        auto& region = mapper.GetRegions()[i];
        // Find corresponding data by name
        const uint8_t* src = nullptr;
        if (strstr(region.name, "DATA")) src = b43AF0000;
        else if (strstr(region.name, "CODE")) src = b34E10000;
        else if (strstr(region.name, "CRT")) src = b7C4A0000;
        else if (strstr(region.name, "STRING")) src = b76ED0000;
        
        if (!mapper.MapRegion(region, src)) {
            LOG_ERROR("Mapper failed: %s", region.name);
            mapper.UnmapAll();
            return false;
        }
    }
    
    GlobalState::Instance().mapper = &mapper;
    
    if (!mapper.Verify()) {
        LOG_ERROR("Mapper verification failed");
        return false;
    }
    
    LOG_SUCCESS("Mapper: all regions OK");
    return true;
}

void FixImports() {
    LOG_INFO("Fixing imports (%zu)...", imports.size());
    int fixed = 0, failed = 0;
    for (auto& [addr, modFunc] : imports) {
        HMODULE mod = LoadLibraryA(modFunc.first);
        if (!mod) mod = GetModuleHandleA(modFunc.first);
        if (!mod) { failed++; continue; }
        FARPROC proc = GetProcAddress(mod, modFunc.second);
        if (!proc) { failed++; continue; }
        *(FARPROC*)addr = proc;
        fixed++;
    }
    LOG_SUCCESS("Imports: %d ok, %d failed", fixed, failed);
}

void FixAddresses() {
    LOG_INFO("Fixing patterns (%zu)...", patterns.size());
    for (auto& [addr, modPat] : patterns) {
        uintptr_t found = PatternScanner::FindOrZero(modPat.first, modPat.second);
        if (!found) { LOG_WARN("Pattern not found: %s %s", modPat.first, modPat.second); continue; }
        *(uintptr_t*)addr = found;
    }
    uintptr_t pHud = PatternScanner::FindOrZero("client.dll", "B9 ? ? ? ? 56 68 ? ? ? ? 89 45");
    if (pHud) *(PDWORD)0x43AFEFA4 = *(PDWORD)(pHud + 1);
    uintptr_t predSeed = PatternScanner::FindOrZero("client.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04");
    if (predSeed) *(PDWORD)0x43AFEE5C = *(DWORD*)(predSeed + 2);
    uintptr_t smokeCount = PatternScanner::FindOrZero("client.dll", "A3 ? ? ? ? 57 8B CB");
    if (smokeCount) *(PDWORD)0x43AFF0B4 = *(DWORD*)(smokeCount + 1);
    uintptr_t nameSpam = PatternScanner::FindOrZero("engine.dll", "38 05 ? ? ? ? 75 ? 8B CE C6 05 ? ? ? ? ? E8 ? ? ? ? C6 05 ? ? ? ? 00");
    if (nameSpam) *(PDWORD)0x43AFF004 = *(DWORD*)(nameSpam + 2);
    DWORD inputBase = *(PDWORD)0x43AFF06C;
    if (inputBase) {
        *(PDWORD)0x43AFEF4C = inputBase + 0xA8;
        *(PDWORD)0x43AFEF54 = inputBase + 0xA5;
    }
    uintptr_t traceSimple = PatternScanner::FindOrZero("client.dll", "C7 45 ? ? ? ? ? C7 45 ? 00 00 00 00 FF 50 ? A1");
    if (traceSimple) *(PDWORD)0x43AFF0FC = *(DWORD*)(traceSimple + 3);
    uintptr_t traceSkipTwo = PatternScanner::FindOrZero("client.dll", "C7 44 24 ? ? ? ? ? FF 90 ? ? ? ? 8D 44 24 ? 50 8D 44 24 ? 50 68 ? ? ? ? 8B 55");
    if (traceSkipTwo) *(PDWORD)0x43AFF100 = *(DWORD*)(traceSkipTwo + 4);
    LOG_SUCCESS("Addresses fixed");
}

void FixConvars() {
    auto& state = GlobalState::Instance();
    if (!state.cvars) state.cvars = GetInterface<ICvar>("vstdlib.dll", "VEngineCvar");
    if (!state.cvars) { LOG_ERROR("ICvar not found"); return; }
    int fixed = 0;
    for (auto& [addr, nameIsCmd] : convars) {
        void* ptr = nameIsCmd.second ? (void*)state.cvars->FindCommand(nameIsCmd.first) : (void*)state.cvars->FindVar(nameIsCmd.first);
        if (!ptr) continue;
        *(void**)addr = ptr;
        fixed++;
    }
    LOG_SUCCESS("Convars fixed: %d", fixed);
}

bool InitializeInterfaces() {
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
        if (!iface) continue;
        *(void**)addr = iface;
        fixed++;
    }
    state.client = *(IBaseClientDLL**)(0x43AFF094);
    if (!state.client) return false;
    try { state.client_mode = **(void***)((*(DWORD**)state.client)[10] + 0x5); } catch (...) {}
    state.prediction = *(void**)(0x43AFF0A8);
    state.surface = *(void**)(0x43AFF098);
    state.trace = *(void**)(0x43AFF01C);
    try { *(PDWORD)0x43AFF020 = **(DWORD**)((*(DWORD**)(state.client))[0] + 0x53); } catch (...) {}
    try { *(PDWORD)0x43AFF06C = *reinterpret_cast<DWORD*>((*reinterpret_cast<uintptr_t**>(state.client))[15] + 0x1); } catch (...) {}
    uintptr_t moveHelperSig = PatternScanner::FindOrZero("client.dll", "8B 0D ? ? ? ? 8B 46 08 68");
    if (moveHelperSig) *(PDWORD)0x43AFEFE8 = **(DWORD**)(moveHelperSig + 0x2);
    uintptr_t itemSystemSig = PatternScanner::FindOrZero("client.dll", "A1 ? ? ? ? 85 C0 75 ? A1 ? ? ? ? 56 68");
    if (itemSystemSig) {
        auto getItemSystem = reinterpret_cast<CCStrike15ItemSystem*(*)()>(itemSystemSig);
        *(PDWORD)0x43AFEFD0 = (DWORD)getItemSystem();
    }
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
    LOG_SUCCESS("Interfaces: %d fixed", fixed);
    return fixed > 0;
}

bool InitializeNetvars() {
    auto& state = GlobalState::Instance();
    if (!state.client) return false;
    bool result = state.netvars.Initialize(state.client);
    LOG_SUCCESS("Netvars: %zu tables", state.netvars.tables.size());
    return result;
}

void FixPostOEP() {
    auto& state = GlobalState::Instance();
    AwRender* render = *(AwRender**)(0x43B01224);
    if (render) {
        render->DidCreateFont = false;
        render->Width = 0;
        render->Height = 0;
    }
    // XOR patches
    for (auto addr : xor_patches) {
        DWORD old;
        VirtualProtect((LPVOID)addr, 1, PAGE_EXECUTE_READWRITE, &old);
        *(BYTE*)addr = 0x75;
        VirtualProtect((LPVOID)addr, 1, old, &old);
    }
    HWND window = nullptr;
    int attempts = 0;
    while (!(window = FindWindowA("Valve001", nullptr)) && attempts < 100) { Sleep(100); attempts++; }
    state.window = window ? window : GetForegroundWindow();
    AwSkinChangerData* skinCtx = *(AwSkinChangerData**)(0x43AF7700);
    if (skinCtx) { memset(skinCtx, 0, 216); state.skinchanger_ctx = skinCtx; }
    state.netvars.GetProp("CCSPlayer", "m_angEyeAngles[1]", (RecvProp**)0x43B01318);
    state.netvars.GetProp("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", (RecvProp**)0x43B01324);
    *(PDWORD)0x43AFE0C4 = (DWORD)state.netvars.GetClass("CBaseWeaponWorldModel");
    *(PDWORD)0x43AFE0E0 = (DWORD)state.netvars.GetClass("CBaseViewModel");
    MODULEINFO info;
    if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("client.dll"), &info, sizeof(info))) {
        *(PDWORD)0x43AFF038 = (DWORD)info.lpBaseOfDll + 0x1000;
        *(PDWORD)0x43AFF024 = info.SizeOfImage;
    }
    // Lag records
    size_t lagSize = 0x3234 * 64;
    void* records = VirtualAlloc(nullptr, lagSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (records) {
        memset(records, 0, lagSize);
        *(PDWORD)0x43AF7A84 = (DWORD)records;
    }
    *(PDWORD)0x43AFE638 = 0;
    struct profile_t { int xor_key = 0; int pad; wchar_t config_path[260]; int pad2[64]; };
    profile_t* profile_ = (profile_t*)VirtualAlloc(nullptr, sizeof(profile_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    memset(profile_, 0, sizeof(profile_t));
    profile_->xor_key = 0;
    memcpy(profile_->config_path, L"\\??\\C:\\aimware\\", sizeof(L"\\??\\C:\\aimware\\"));
    *(PDWORD)0x43AFF218 = (DWORD)profile_;
    CreateDirectoryA("C:\\aimware\\", 0);
    ConfigSystem::Instance().Initialize(L"C:\\aimware\\");
    // NOP checks
    auto nopRange = [](uintptr_t start, uintptr_t end) {
        DWORD old; VirtualProtect((LPVOID)start, end-start, PAGE_EXECUTE_READWRITE, &old);
        memset((void*)start, 0x90, end-start);
        VirtualProtect((LPVOID)start, end-start, old, &old);
    };
    nopRange(0x34E1DA71, 0x34E1DA82);
    nopRange(0x34E1DA85, 0x34E1DA8A);
    nopRange(0x34E1DA8F, 0x34E1DA94);
    nopRange(0x34E1DA9A, 0x34E1DA9D);
    nopRange(0x34E1DAA2, 0x34E1DAA5);
    nopRange(0x34E1DAA8, 0x34E1DAAD);
    try { ((void(*)())(0x34E1D890))(); } catch (...) {}
    try { ((void(*)())(0x34E2B410))(); } catch (...) {}
    memset((void*)0x43AF8B2C, 0, 32);
    LOG_SUCCESS("Post-OEP done");
}

void HookNetvar(const char* table, const char* var, uintptr_t original_addr, uintptr_t hook_fn) {
    auto& state = GlobalState::Instance();
    RecvProp* prop = nullptr;
    state.netvars.GetProp(table, var, &prop);
    if (!prop) return;
    state.hooked_netvars.push_back({ (uintptr_t)prop->proxy, (uintptr_t)prop });
    if (original_addr) *(void**)original_addr = (void*)prop->proxy;
    prop->proxy = (RecvVarProxyFn)hook_fn;
}

void UnhookNetvars() {
    auto& state = GlobalState::Instance();
    for (auto& [original, propAddr] : state.hooked_netvars) {
        ((RecvProp*)propAddr)->proxy = (RecvVarProxyFn)original;
    }
    state.hooked_netvars.clear();
}

void __cdecl hkPanic() {
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
}

void __stdcall hkGenConfigPath(const char* name, wchar_t* out) {
    Aimware2016Decompiled::ConfigUIEngine::hkGenConfigPath(name, out);
}

bool InitializeHooks() {
    auto& state = GlobalState::Instance();
    DetourFunction((PBYTE)0x34E34E90, (PBYTE)hkGenConfigPath);
    DetourFunction((PBYTE)0x34E25530, (PBYTE)hkPanic);
    if (state.window) {
        state.orig_wndproc = (WNDPROC)SetWindowLongPtr(state.window, GWL_WNDPROC, (LONG_PTR)((void*)0x34E33D60));
        *(WNDPROC*)(0x43AFF104) = state.orig_wndproc;
        *(HWND*)(0x43AFF214) = state.window;
    }
#ifdef USE_DECOMPILED_ENGINE
    if (state.engine_vgui_hook) *(PDWORD)0x43AFE63C = state.engine_vgui_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkEngineVGUIPaint, 14);
    if (state.client_hook) {
        *(PDWORD)0x43AFE630 = state.client_hook->HookFunction(0x34E26330, 36);
        state.client_hook->HookFunction(0x34E268C0, 23);
    }
    if (state.client_mode_hook) *(PDWORD)0x43AFE178 = state.client_mode_hook->HookFunction((DWORD)&Aimware2016Decompiled::AimbotEngine::hkCreateMove, 24);
    if (state.prediction_hook) {
        *(PDWORD)0x43AFEE54 = state.prediction_hook->HookFunction(0x34E314B0, 19);
        *(PDWORD)0x43AFE644 = state.prediction_hook->HookFunction(0x34E26880, 20);
        state.prediction_hook->HookFunction(0x34E26500, 14);
    }
    if (state.surface_hook) *(PDWORD)0x43AFE634 = state.surface_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkLockCursor, 67);
    if (state.studio_render_hook) *(PDWORD)0x43AFE17C = state.studio_render_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkDrawModel, 29);
    *(PDWORD)0x43AFE0E8 = (DWORD)DetourFunction((PBYTE)PatternScanner::FindOrZero("client.dll", "55 8B EC 83 EC ? 56 8B F1 57 89 75 ? E8 ? ? ? ? 8B CE E8"), (PBYTE)&Aimware2016Decompiled::ResolverEngine::hkOnRenderStart);
    HookNetvar("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", 0x43B01328, 0x34E24BC0);
    HookNetvar("CCSPlayer", "m_angEyeAngles[0]", 0, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnPitchProxy);
    HookNetvar("CCSPlayer", "m_angEyeAngles[1]", 0, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnYawProxy);
    HookNetvar("CCSPlayer", "m_flThirdpersonRecoil", 0x43B01304, 0x34E24FC0);
    HookNetvar("CCSPlayer", "m_flLowerBodyYawTarget", 0x43B01334, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnLBYProxy);
    HookNetvar("CBasePlayer", "m_fFlags", 0x43B01340, 0x34E24F40);
    HookNetvar("CCSPlayer", "m_flFlashDuration", 0x43B0134C, 0x34E24B90);
#else
    if (state.engine_vgui_hook) *(PDWORD)0x43AFE63C = state.engine_vgui_hook->HookFunction(0x34E26660, 14);
    if (state.client_hook) { *(PDWORD)0x43AFE630 = state.client_hook->HookFunction(0x34E26330, 36); state.client_hook->HookFunction(0x34E268C0, 23); }
    if (state.client_mode_hook) *(PDWORD)0x43AFE178 = state.client_mode_hook->HookFunction(0x34E258A0, 24);
    if (state.prediction_hook) { *(PDWORD)0x43AFEE54 = state.prediction_hook->HookFunction(0x34E314B0, 19); *(PDWORD)0x43AFE644 = state.prediction_hook->HookFunction(0x34E26880, 20); state.prediction_hook->HookFunction(0x34E26500, 14); }
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
    LOG_SUCCESS("Hooks OK");
    return true;
}

void Shutdown() {
    auto& state = GlobalState::Instance();
    state.Reset();
    LOG_INFO("Shutdown");
}

} // namespace Aimware

DWORD WINAPI install_thread(PVOID) {
    Aimware::Logger::Instance().Initialize(true, false);
    LOG_INFO("=== Aimware 2016 Loader ===");
    LOG_INFO("Mapper: fixed regions 0x34E10000, 0x43AF0000, 0x76ED0000, 0x7C4A0000");
    
    while (!GetModuleHandleA("serverbrowser.dll")) Sleep(100);
    Sleep(500);

    if (!Aimware::InitializeMemoryDumps()) { LOG_ERROR("Mapper failed"); return 1; }

    auto& state = GlobalState::Instance();
    state.render = *(AwRender**)(0x43B01224);
    state.global_ctx = *(AwGlobals**)(0x43AF7704);
    state.skinchanger_ctx = *(AwSkinChangerData**)(0x43AF7700);

    Aimware::FixImports();
    Aimware::InitializeInterfaces();
    Aimware::InitializeNetvars();
    Aimware::FixAddresses();
    Aimware::FixConvars();
    Aimware::FixPostOEP();
    Aimware2016Decompiled::InitializeDecompiledEngine();
    Aimware::InitializeHooks();

    state.initialized = true;
    LOG_SUCCESS("=== Aimware Loaded ===");
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
