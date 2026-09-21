#include "aw.h"
#include "detours.h"

#define CSGO2016

using namespace Aimware;
using namespace Aimware::Utils;
using namespace Aimware::Reversed;

bool call_in_bounds(void* addr) {
    uintptr_t u = (uintptr_t)addr;
    return u > 0x34E10000 && u < (0x34E10000 + sizeof(b34E10000));
}

// ===================== Import Table (Full Reverse) =====================
std::unordered_map<DWORD, std::pair<const char*, const char*>> imports = {
    // Kernel32 - 100% restored from dump_analyzer
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

    // User32 - 90% restored, plus extras
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

    // MSVCRT - 100% restored
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

    // NTDLL - 100% restored
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

// Interface table - full reverse from dump_analyzer
std::unordered_map<DWORD, std::pair<const char*, const char*>> interfaces = {
    { DataSection::VENGINE_CLIENT, { "engine.dll", "VEngineClient" } },
    { DataSection::ENGINE_TRACE, { "engine.dll", "EngineTraceClient" } },
    { DataSection::MODEL_INFO, { "engine.dll", "VModelInfoClient0" } },
    { DataSection::DEBUG_OVERLAY, { "engine.dll", "VDebugOverlay" } },
    { DataSection::VCLIENT, { "client.dll", "VClient" } },
    { DataSection::ENTITY_LIST, { "client.dll", "VClientEntityList" } },
    { DataSection::PREDICTION, { "client.dll", "VClientPrediction0" } },
    { DataSection::GAME_MOVEMENT, { "client.dll", "GameMovement0" } },
    { DataSection::MATERIAL_SYSTEM, { "materialsystem.dll", "VMaterialSystem" } },
    { DataSection::SURFACE, { "vguimatsurface.dll", "VGUI_Surface" } },
    { DataSection::PHYSICS_PROPS, { "vphysics.dll", "VPhysicsSurfaceProps" } },
    { DataSection::RANDOM_FLOAT, { "vstdlib.dll", "RandomFloat" } },
    { DataSection::RANDOM_INT, { "vstdlib.dll", "RandomInt" } },
    { DataSection::RANDOM_SEED, { "vstdlib.dll", "RandomSeed" } },
    { DataSection::ENGINE_CVAR, { "vstdlib.dll", "VEngineCvar" } },
    { DataSection::LOCALIZE, { "localize.dll", "Localize_" } },
    { DataSection::MDL_CACHE, { "datacache.dll", "MDLCache" } },
};

// Pattern table - full reverse
std::unordered_map<DWORD, std::pair<const char*, const char*>> patterns = {
    { DataSection::WRITE_USERCMD, { "client.dll", "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" } },
    { DataSection::FINISH_DRAWING, { "vguimatsurface.dll", "8B 0D ? ? ? ? 56 C6 05" } },
    { DataSection::START_DRAWING, { "vguimatsurface.dll", "55 8B EC 83 E4 ? 83 EC ? 80 3D" } },
    { DataSection::IS_BREAKABLE, { "client.dll", "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE" } },
#ifndef CSGO2018
    { DataSection::GET_WEAPON_INFO, { "client.dll", "66 3B 0D ? ? ? ? 72" } },
    { DataSection::FIND_HUD_ELEMENT, { "client.dll", "55 8B EC 51 8B C1 53 56 8B 75" } },
    { DataSection::MD5_PSEUDORANDOM, { "client.dll", "55 8B EC 83 E4 ? 83 EC ? 6A ? 8D 44 24" } },
    { DataSection::GLOW_MANAGER, { "client.dll", "A1 ? ? ? ? A8 ? 75 ? 0F 57 C0 C7 05 ? ? ? ? 00 00 00 00 F3 0F 7F 05 ? ? ? ? 83 C8 ? C7 05 ? ? ? ? 00 00 00 00 66 0F 6F 05 ? ? ? ? 68 ? ? ? ? A3 ? ? ? ? F3 0F 7F 05 ? ? ? ? C7 05 ? ? ? ? 00 00 00 00 E8 ? ? ? ? 83 C4 ? B8" } },
#else
    { DataSection::GET_WEAPON_INFO, { "client.dll", "55 8B EC 81 EC ? ? ? ? 53 8B D9 56 57 8D 8B ? ? ? ? 85 C9 75 04 33 FF EB 2F" } },
    { DataSection::FIND_HUD_ELEMENT, { "client.dll", "55 8B EC 53 8B 5D ? 56 57 8B F9 33 F6 39 77 ? 7E ? 8B 47 ? ? ? ? ? ? FF 50" } },
    { DataSection::MD5_PSEUDORANDOM, { "client.dll", "55 8B EC 83 E4 ? 83 EC ? 6A ? 8D 44 24 ? 89 4C 24" } },
    { DataSection::GLOW_MANAGER, { "client.dll", "A1 ? ? ? ? A8 01 75 4B" } },
#endif
    { DataSection::CLIP_TRACE, { "client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10" } },
};

std::unordered_map<DWORD, std::pair<const char*, bool>> convars = {
    { NetvarStorage::M_YAW, { "m_yaw", false } },
    { NetvarStorage::M_PITCH, { "m_pitch", false } },
    { NetvarStorage::SENSITIVITY, { "sensitivity", false } },
    { NetvarStorage::SV_CHEATS, { "sv_cheats", false } },
    { NetvarStorage::SV_FOOTSTEPS, { "sv_footsteps", false } },
    { NetvarStorage::CL_MODELFASTPATH, { "cl_modelfastpath", false } },
    { NetvarStorage::WEAPON_RECOIL_SCALE, { "weapon_recoil_scale", false } },
    { NetvarStorage::SV_GRAVITY, { "sv_gravity", false } },
    { NetvarStorage::CL_INTERPOLATE, { "cl_interpolate", false } },
    { NetvarStorage::VIEW_RECOIL_TRACKING, { "view_recoil_tracking", false } },
    { NetvarStorage::SV_MAXUPDATERATE, { "sv_maxupdaterate", false } },
    { NetvarStorage::CL_INTERP, { "cl_interp", false } },
    { NetvarStorage::SV_MINUPDATERATE, { "sv_minupdaterate", false } },
    { NetvarStorage::CL_INTERP_RATIO, { "cl_interp_ratio", false } },
    { NetvarStorage::SV_CLIENT_MAX_INTERP_RATIO, { "sv_client_max_interp_ratio", false } },
    { NetvarStorage::CL_UPDATERATE, { "cl_updaterate", false } },
    { NetvarStorage::SV_CLIENT_MIN_INTERP_RATIO, { "sv_client_min_interp_ratio", false } },
    { NetvarStorage::MOLOTOV_THROW_DETONATE, { "molotov_throw_detonate_time", false } },
    { NetvarStorage::MOLOTOV_MAX_SLOPE, { "weapon_molotov_maxdetonateslope", false } },
    { NetvarStorage::NAME, { "name", false } },
    { NetvarStorage::R_3DSKY, { "r_3dsky", false } },
    { NetvarStorage::R_DRAWSKYBOX, { "r_drawskybox", false } },
    { NetvarStorage::GL_CLEAR, { "gl_clear", false } },
    { NetvarStorage::CL_FULLUPDATE, { "cl_fullupdate", true } },
};

// XOR patches - full list from reversed/memory_map.hpp
std::vector<DWORD> xor_patches = {
    // Config Save & Parsing
    0x34E1EAAD, 0x34E1EB16, 0x34E1EB86, 0x34E2B4BB, 0x34E2B473,
    // ESP & Visuals
    0x34E1FE4A, 0x34E20169, 0x34E208A5, 0x34E20995, 0x34E20A8A, 0x34E20BC6, 0x34E20D55, 0x34E20E2F,
    0x34E239C7, 0x34E1F584, 0x34E1F26A, 0x34E1F2B5, 0x34E1F2F8, 0x34E1F002, 0x34E1F045, 0x34E1F088,
    // SkinChanger & GloveChanger
    0x34E1DFC9, 0x34E1E2DA, 0x34E1E32A, 0x34E1E38E, 0x34E1F14E, 0x34E1F190, 0x34E1F1D0,
    // DrawModel Chams
    0x34E25A13,
    // Chat & Name Spam
    0x34E2AD02, 0x34E2AC29,
    // Weapons & Fonts & Radar
    0x34E30DE4, 0x34E31D7B, 0x34E30FE4,
    // UI Elements
    0x34E37D73, 0x34E3791E, 0x34E38185, 0x34E3828B, 0x34E382D3, 0x34E388A4, 0x34E389AF, 0x34E38A52,
    0x34E38FB2, 0x34E39396, 0x34E372C5, 0x34E37074, 0x34E36CC5, 0x34E36C45, 0x34E36BC5, 0x34E36B45,
    // Config
    0x34E35983, 0x34E357C5, 0x34E34746, 0x34E37EEF, 0x34E3837E, 0x34E38C48, 0x34E394BF, 0x34E39C5B,
    0x34E3783F, 0x34E3774E, 0x34E36A6F, 0x34E3640E, 0x34E35028, 0x34E350C1, 0x34E34D58, 0x34E34C1A,
    0x34E34C4C, 0x34E343AA, 0x34E34464, 0x34E2B347, 0x34E2B2EA,
};

// ===================== Advanced Mapper Implementation =====================

namespace Aimware {

bool InitializeMemoryDumpsAdvanced() {
    LOG_INFO("=== Advanced Mapper Initialization ===");
    
    auto& mapper = Mapping::AdvancedMapper::Instance();
    auto& state = GlobalState::Instance();
    state.mapper = &mapper;
    state.useAdvancedMapper = true;

    // Analyze dumps before mapping
    LOG_INFO("Analyzing dumps with PE parser...");
    
    // Add regions with full info
    mapper.AddRegion(Bases::CRT_BASE, Bases::CRT_SIZE, "CRT Helper (b7C4A0000)", b7C4A0000, PAGE_EXECUTE_READWRITE, false);
    mapper.AddRegion(Bases::STRING_BASE, Bases::STRING_SIZE, "String Resources (b76ED0000)", b76ED0000, PAGE_READWRITE, false);
    mapper.AddRegion(Bases::DATA_BASE, Bases::DATA_SIZE, "Data Section (b43AF0000)", b43AF0000, PAGE_READWRITE, false);
    mapper.AddRegion(Bases::CODE_BASE, Bases::CODE_SIZE, "Code Section (b34E10000)", b34E10000, PAGE_EXECUTE_READWRITE, true);

    // Detailed analysis
    for (auto& region : mapper.GetRegions()) {
        if (region.sourceData) {
            auto analysis = PE::AimwareDumpAnalyzer::Analyze(
                region.preferredBase, region.size, region.name, region.sourceData);
            region.analysis = analysis;
        }
    }

    // Map all
    if (!mapper.MapAll(true)) {
        LOG_ERROR("Advanced mapper failed, falling back to basic");
        return InitializeMemoryDumps();
    }

    LOG_SUCCESS("Advanced mapper: all regions mapped");
    
    // Verify
    mapper.VerifyMapping();
    
    return true;
}

bool InitializeMemoryDumps() {
#ifdef USE_ADVANCED_MAPPER
    return InitializeMemoryDumpsAdvanced();
#else
    LOG_INFO("Initializing memory dumps (basic)...");
    auto& mem = MemoryManager::Instance();

    if (!mem.AllocateFixed(Bases::CRT_BASE, Bases::CRT_SIZE, "CRT Helper", b7C4A0000)) return false;
    if (!mem.AllocateFixed(Bases::STRING_BASE, Bases::STRING_SIZE, "String Resources", b76ED0000)) return false;
    if (!mem.AllocateFixed(Bases::DATA_BASE, Bases::DATA_SIZE, "Data Section", b43AF0000)) return false;
    if (!mem.AllocateFixed(Bases::CODE_BASE, Bases::CODE_SIZE, "Code Section", b34E10000)) return false;

    LOG_SUCCESS("All dumps allocated (basic)");
    return true;
#endif
}

void AnalyzeDumps() {
    LOG_INFO("=== Full Dump Analysis ===");
    
    struct DumpInfo {
        uintptr_t base;
        size_t size;
        const char* name;
        const uint8_t* data;
    } dumps[] = {
        { Bases::CODE_BASE, Bases::CODE_SIZE, "CODE", b34E10000 },
        { Bases::DATA_BASE, Bases::DATA_SIZE, "DATA", b43AF0000 },
        { Bases::STRING_BASE, Bases::STRING_SIZE, "STRING", b76ED0000 },
        { Bases::CRT_BASE, Bases::CRT_SIZE, "CRT", b7C4A0000 },
    };

    for (auto& dump : dumps) {
        auto analysis = PE::AimwareDumpAnalyzer::Analyze(dump.base, dump.size, dump.name, dump.data);
        PE::AimwareDumpAnalyzer::PrintAnalysis(analysis);
    }
}

bool DecryptStringSection() {
    LOG_INFO("=== Decrypting String Section ===");
    
    auto& state = GlobalState::Instance();
    uintptr_t profilePtr = *(uintptr_t*)DataSection::PROFILE_CONTEXT;
    if (!profilePtr) {
        LOG_WARN("Profile context not yet initialized, using key 0");
        state.profileXorKey = 0;
        return false;
    }

    int xorKey = *(int*)profilePtr;
    state.profileXorKey = xorKey;
    LOG_INFO("Profile XOR key: 0x%08X (%d)", xorKey, xorKey);

    if (xorKey == 0) {
        LOG_WARN("XOR key is 0, strings may use different decryption or not encrypted");
        // Try to brute-force
        uint8_t* stringSection = (uint8_t*)Bases::STRING_BASE;
        auto encType = Crypto::Decryptor::DetectEncryption(stringSection, Bases::STRING_SIZE);
        LOG_INFO("Detected encryption type for STRING section: %d", (int)encType);
    }

    // Decrypt string section
    uint8_t* stringSection = (uint8_t*)Bases::STRING_BASE;
    if (stringSection) {
        auto strings = Crypto::StringDecryptor::DecryptAllStrings(
            stringSection, Bases::STRING_SIZE, Bases::STRING_BASE, xorKey);
        state.decryptedStrings = strings;
        LOG_SUCCESS("Decrypted %zu strings from STRING section", strings.size());
        for (size_t i = 0; i < std::min(strings.size(), size_t(10)); ++i) {
            LOG_INFO("String 0x%08X: %s", strings[i].address, strings[i].decrypted.c_str());
        }
    }

    return true;
}

void FixXorPatches() {
    LOG_INFO("=== Fixing XOR Patches (%zu) ===", xor_patches.size());
    
#ifdef USE_ADVANCED_MAPPER
    auto& mapper = Mapping::AdvancedMapper::Instance();
    auto codeRegion = mapper.GetRegion("Code Section (b34E10000)");
    if (codeRegion && codeRegion->isMapped) {
        // Use advanced mapper's patching
        for (auto addr : xor_patches) {
            auto region = mapper.GetRegionByAddress(addr);
            if (region && region->isMapped) {
                uint8_t* ptr = (uint8_t*)addr;
                DWORD old;
                VirtualProtect(ptr, 1, PAGE_EXECUTE_READWRITE, &old);
                *ptr = 0x75; // JNZ
                VirtualProtect(ptr, 1, old, &old);
            }
        }
    } else
#endif
    {
        auto& mem = MemoryManager::Instance();
        for (auto addr : xor_patches) {
            mem.PatchByte(addr, 0x75);
        }
    }
    
    LOG_SUCCESS("XOR patches applied: %zu", xor_patches.size());
}

void FixImports() {
    LOG_INFO("Fixing imports (%zu entries) with full reverse...", imports.size());

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

void FixImportsAdvanced() {
    LOG_INFO("=== Advanced Import Fixing ===");
    FixImports();
    
    // Additional verification: check that import table is fully resolved
    uintptr_t importTable = Bases::IMPORT_TABLE;
    int nullCount = 0;
    for (size_t i = 0; i < Bases::IMPORT_TABLE_SIZE; i += 4) {
        uintptr_t* ptr = (uintptr_t*)(importTable + i);
        if (*ptr == 0) nullCount++;
    }
    LOG_INFO("Import table null entries: %d/%d", nullCount, Bases::IMPORT_TABLE_SIZE/4);
}

void FixAddresses() {
    LOG_INFO("Fixing addresses (%zu patterns) with full reverse...", patterns.size());

    for (auto& [addr, modPat] : patterns) {
        uintptr_t found = PatternScanner::FindOrZero(modPat.first, modPat.second);
        if (!found) {
            LOG_WARN("Pattern not found: %s [%s]", modPat.first, modPat.second);
            continue;
        }
        *(uintptr_t*)addr = found;
        LOG_INFO("Pattern %s -> 0x%08X at 0x%08X", modPat.first, found, addr);
    }

    // pHud
    uintptr_t pHudSig = PatternScanner::FindOrZero("client.dll", "B9 ? ? ? ? 56 68 ? ? ? ? 89 45");
    if (pHudSig) {
        *(PDWORD)DataSection::HUD_PTR = *(PDWORD)(pHudSig + 1);
        LOG_INFO("pHud fixed: 0x%08X", *(PDWORD)DataSection::HUD_PTR);
    }

    // PredictionRandomSeed
    uintptr_t predSeed = PatternScanner::FindOrZero("client.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04");
    if (predSeed) {
        *(PDWORD)DataSection::PREDICTION_RANDOM_SEED = *(DWORD*)(predSeed + 2);
        LOG_INFO("PredictionRandomSeed fixed");
    }

    // SmokeCount
    uintptr_t smokeCount = PatternScanner::FindOrZero("client.dll", "A3 ? ? ? ? 57 8B CB");
    if (smokeCount) {
        *(PDWORD)DataSection::SMOKE_COUNT = *(DWORD*)(smokeCount + 1);
        LOG_INFO("SmokeCount fixed: 0x%08X", *(PDWORD)DataSection::SMOKE_COUNT);
    }

    // Name spam
    uintptr_t nameSpam = PatternScanner::FindOrZero("engine.dll", "38 05 ? ? ? ? 75 ? 8B CE C6 05 ? ? ? ? ? E8 ? ? ? ? C6 05 ? ? ? ? 00");
    if (nameSpam) {
        *(PDWORD)DataSection::NAMESTEALER = *(DWORD*)(nameSpam + 2);
    }

    // CInput offsets - full reverse: CINPUT + 0xA8 and +0xA5
    DWORD inputBase = *(PDWORD)DataSection::CINPUT;
    if (inputBase) {
        *(PDWORD)DataSection::CAMERA_IN_THIRD_PERSON = inputBase + 0xA8;
        *(PDWORD)DataSection::VIEW_OFFSET = inputBase + 0xA5;
        LOG_INFO("CInput offsets: thirdperson=0x%08X view=0x%08X", 
            *(PDWORD)DataSection::CAMERA_IN_THIRD_PERSON, *(PDWORD)DataSection::VIEW_OFFSET);
    }

    // Trace filters
    uintptr_t traceSimple = PatternScanner::FindOrZero("client.dll", "C7 45 ? ? ? ? ? C7 45 ? 00 00 00 00 FF 50 ? A1");
    if (traceSimple) {
        *(PDWORD)DataSection::TRACE_FILTER_SIMPLE = *(DWORD*)(traceSimple + 3);
    }

    uintptr_t traceSkipTwo = PatternScanner::FindOrZero("client.dll", "C7 44 24 ? ? ? ? ? FF 90 ? ? ? ? 8D 44 24 ? 50 8D 44 24 ? 50 68 ? ? ? ? 8B 55");
    if (traceSkipTwo) {
        *(PDWORD)DataSection::TRACE_FILTER_SKIP_TWO = *(DWORD*)(traceSkipTwo + 4);
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
    LOG_INFO("Initializing interfaces (%zu entries) with full reverse...", interfaces.size());

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

    // Resolve additional interfaces - full reverse
    state.client = *(IBaseClientDLL**)(DataSection::VCLIENT);
    if (!state.client) {
        LOG_ERROR("Client interface null");
        return false;
    }

    // ClientMode - from client vtable[10] + 5 - verified from disasm
    try {
        state.client_mode = **(void***)((*(DWORD**)state.client)[10] + 0x5);
        LOG_INFO("ClientMode: 0x%p (from client[10]+5)", state.client_mode);
    } catch (...) {
        LOG_ERROR("Failed to get ClientMode");
    }

    state.prediction = *(void**)(DataSection::PREDICTION);
    state.surface = *(void**)(DataSection::SURFACE);
    state.trace = *(void**)(DataSection::ENGINE_TRACE);

    // GlobalVarsBase - version dependent
    try {
#ifndef CSGO2018
        *(PDWORD)DataSection::GLOBAL_VARS = **(DWORD**)((*(DWORD**)(state.client))[0] + 0x53);
        LOG_INFO("GlobalVarsBase (2016) fixed: 0x%08X", *(PDWORD)DataSection::GLOBAL_VARS);
#else
        *(PDWORD)DataSection::GLOBAL_VARS = **(DWORD**)((*(DWORD**)(state.client))[0] + 0x1B);
        LOG_INFO("GlobalVarsBase (2018) fixed: 0x%08X", *(PDWORD)DataSection::GLOBAL_VARS);
#endif
    } catch (...) {
        LOG_ERROR("Failed to fix GlobalVarsBase");
    }

    // CInput - from client vtable[15] + 1
    try {
        *(PDWORD)DataSection::CINPUT = *reinterpret_cast<DWORD*>((*reinterpret_cast<uintptr_t**>(state.client))[15] + 0x1);
        LOG_INFO("CInput fixed: 0x%08X", *(PDWORD)DataSection::CINPUT);
    } catch (...) {
        LOG_ERROR("Failed to fix CInput");
    }

    // MoveHelper
    uintptr_t moveHelperSig = PatternScanner::FindOrZero("client.dll", "8B 0D ? ? ? ? 8B 46 08 68");
    if (moveHelperSig) {
        *(PDWORD)DataSection::MOVE_HELPER = **(DWORD**)(moveHelperSig + 0x2);
        LOG_INFO("MoveHelper fixed");
    }

    // ItemSystem
    uintptr_t itemSystemSig = PatternScanner::FindOrZero("client.dll", "A1 ? ? ? ? 85 C0 75 ? A1 ? ? ? ? 56 68");
    if (itemSystemSig) {
        auto getItemSystem = reinterpret_cast<CCStrike15ItemSystem*(*)()>(itemSystemSig);
        *(PDWORD)DataSection::ITEM_SYSTEM = (DWORD)getItemSystem();
        LOG_INFO("ItemSystem fixed: 0x%08X", *(PDWORD)DataSection::ITEM_SYSTEM);
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
    LOG_INFO("Initializing netvars with full reverse...");

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
    
    // Dump important netvars
    LOG_INFO("Dumping critical netvars:");
    const char* critical[][2] = {
        {"CCSPlayer", "m_angEyeAngles[0]"},
        {"CCSPlayer", "m_angEyeAngles[1]"},
        {"CCSPlayer", "m_flLowerBodyYawTarget"},
        {"CCSPlayer", "m_flFlashDuration"},
        {"CBasePlayer", "m_fFlags"},
        {"CCSPlayer", "m_iHealth"},
        {"CBaseCombatWeapon", "m_iItemDefinitionIndex"},
    };
    
    for (auto& [table, prop] : critical) {
        int offset = state.netvars.GetOffset(table, prop);
        LOG_INFO("  %s::%s = 0x%04X", table, prop, offset);
    }

    return true;
}

void FixPostOEP() {
    LOG_INFO("Fixing post-OEP with full reverse...");

    auto& state = GlobalState::Instance();
    
#ifdef USE_ADVANCED_MAPPER
    auto& mapper = Mapping::AdvancedMapper::Instance();
    auto& mem = MemoryManager::Instance();
#else
    auto& mem = MemoryManager::Instance();
#endif

    // Fix render - from reversed
    AwRender* render = *(AwRender**)(DataSection::RENDER_CONTEXT_PTR);
    if (render) {
        LOG_INFO("Render original res: %dx%d", render->Width, render->Height);
        render->DidCreateFont = false;
        render->Width = 0;
        render->Height = 0;
    }

    // Fix XOR patches
    FixXorPatches();

    // Find window - Valve001
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

    // Reset skinchanger struct - 216 bytes
    AwSkinChangerData* skinCtx = *(AwSkinChangerData**)(DataSection::SKINCHANGER_CONTEXT);
    if (skinCtx) {
        memset(skinCtx, 0, 216);
        state.skinchanger_ctx = skinCtx;
        LOG_INFO("Skinchanger context reset");
    }

    // Fix netvar proxies - full reverse
    state.netvars.GetProp("CCSPlayer", "m_angEyeAngles[1]", (RecvProp**)DataSection::M_ANG_EYE_ANGLES_1_PROXY);
    state.netvars.GetProp("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", (RecvProp**)DataSection::M_SMOKE_TICK_PROXY);

    *(PDWORD)DataSection::BASE_WEAPON_WORLD_MODEL_CLASS = (DWORD)state.netvars.GetClass("CBaseWeaponWorldModel");
    *(PDWORD)DataSection::BASE_VIEW_MODEL_CLASS = (DWORD)state.netvars.GetClass("CBaseViewModel");

    // Client.dll info for internal pattern scanner
    MODULEINFO info;
    if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("client.dll"), &info, sizeof(info))) {
        *(PDWORD)DataSection::CLIENT_BASE = (DWORD)info.lpBaseOfDll + 0x1000;
        *(PDWORD)DataSection::CLIENT_SIZE = info.SizeOfImage;
        LOG_INFO("Client.dll base: 0x%p size: 0x%X", info.lpBaseOfDll, info.SizeOfImage);
    }

    // Allocate lag records - 0x3234 * 64
    void* records = mem.AllocateLagRecords();
    if (!records) {
        LOG_ERROR("Failed to allocate lag records");
    } else {
        LOG_SUCCESS("Lag records allocated at 0x%p (0x%X bytes)", records, 0x3234*64);
    }

    *(PDWORD)DataSection::UNLOCK_CURSOR_FLAG = 0;

    // Profile context - full reverse from config_struct.hpp
    struct profile_t {
        int xor_key = 0;
        int pad;
        wchar_t config_path[260];
        int pad2[64];
    };

    profile_t* profile_ = (profile_t*)mem.AllocateDynamic(sizeof(profile_t), "Profile");
    memset(profile_, 0, sizeof(profile_t));
    profile_->xor_key = 0; // Will be used for string decryption
    memcpy(profile_->config_path, L"\\??\\C:\\aimware\\", sizeof(L"\\??\\C:\\aimware\\"));
    *(PDWORD)DataSection::PROFILE_CONTEXT = (DWORD)profile_;
    state.profileXorKey = profile_->xor_key;

    CreateDirectoryA("C:\\aimware\\", 0);
    ConfigSystem::Instance().Initialize(L"C:\\aimware\\");

    // NOP out skinchanger checks - full reverse
    mem.NopRange(0x34E1DA71, 0x34E1DA82);
    mem.NopRange(0x34E1DA85, 0x34E1DA8A);
    mem.NopRange(0x34E1DA8F, 0x34E1DA94);
    mem.NopRange(0x34E1DA9A, 0x34E1DA9D);
    mem.NopRange(0x34E1DAA2, 0x34E1DAA5);
    mem.NopRange(0x34E1DAA8, 0x34E1DAAD);

    // Try to decrypt strings after profile is set
    DecryptStringSection();

#ifdef CSGO2018
    // 2018 fixes
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
    *(PDWORD)Offsets::NetvarStorage::M_F_FLAGS = 0x29BC;
    *(PDWORD)Offsets::CCSPlayer::M_FL_FLASH_DURATION = 0xA310;
    *(PDWORD)Offsets::CCSPlayer::M_ANG_EYE_ANGLES = 0x32B0;
    *(PDWORD)Offsets::CCSPlayer::M_FL_THIRDPERSON_RECOIL = 0x3360;
    // ... more 2018 offsets
#endif

    // Call netvar & skins init - full reverse
    LOG_INFO("Calling netvar init at 0x%08X", Functions::Init::NETVAR_INIT);
    try {
        ((void(*)())(Functions::Init::NETVAR_INIT))();
    } catch (...) {
        LOG_ERROR("Exception in netvar init");
    }

    LOG_INFO("Refreshing config list at 0x%08X", Functions::Init::CONFIG_LIST_REFRESH);
    try {
        ((void(*)())(Functions::Init::CONFIG_LIST_REFRESH))();
    } catch (...) {
        LOG_ERROR("Exception in config list refresh");
    }

    // Reset config name - 32 bytes
    memset((void*)DataSection::CONFIG_PATH, 0, 32);

    LOG_SUCCESS("Post-OEP fixes done with full reverse");
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
    Aimware2016Decompiled::ConfigUIEngine::hkGenConfigPath(name, out);
}

bool InitializeHooks() {
    LOG_INFO("Initializing hooks with full reverse...");

    auto& state = GlobalState::Instance();

    DetourFunction((PBYTE)Functions::ConfigUI::GEN_CONFIG_PATH, (PBYTE)hkGenConfigPath);
    DetourFunction((PBYTE)Functions::Init::PANIC, (PBYTE)hkPanic);

    if (state.window) {
        state.orig_wndproc = (WNDPROC)SetWindowLongPtr(state.window, GWL_WNDPROC, (LONG_PTR)((void*)Functions::Init::WND_PROC));
        *(WNDPROC*)(DataSection::ORIG_WNDPROC) = state.orig_wndproc;
        *(HWND*)(DataSection::WINDOW_HANDLE) = state.window;
        LOG_INFO("WndProc hooked: original 0x%p -> 0x%08X", state.orig_wndproc, Functions::Init::WND_PROC);
    }

#ifdef USE_DECOMPILED_ENGINE
    LOG_INFO("Using DECOMPILED engine hooks (full reverse)");

    if (state.engine_vgui_hook) {
        *(PDWORD)DataSection::ORIG_PAINT = state.engine_vgui_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkEngineVGUIPaint, 14);
        LOG_INFO("Hooked EngineVGUI::Paint [14] -> 0x%p", &Aimware2016Decompiled::VisualsEngine::hkEngineVGUIPaint);
    }
    if (state.client_hook) {
        *(PDWORD)DataSection::ORIG_FRAME_STAGE = state.client_hook->HookFunction(Functions::FrameStageNotify::ADDRESS, 36);
        state.client_hook->HookFunction(Functions::Prediction::WRITE_USERCMD_DELTA, 23);
        LOG_INFO("Hooked FrameStageNotify [36] -> 0x%08X", Functions::FrameStageNotify::ADDRESS);
    }
    if (state.client_mode_hook) {
        *(PDWORD)DataSection::ORIG_CREATE_MOVE = state.client_mode_hook->HookFunction((DWORD)&Aimware2016Decompiled::AimbotEngine::hkCreateMove, 24);
        LOG_INFO("Hooked CreateMove [24] -> decompiled");
    }
    if (state.prediction_hook) {
        *(PDWORD)DataSection::ORIG_RUN_COMMAND = state.prediction_hook->HookFunction(Functions::Prediction::RUN_COMMAND, 19);
        *(PDWORD)DataSection::ORIG_SETUP_MOVE = state.prediction_hook->HookFunction(Functions::Prediction::SETUP_MOVE, 20);
        state.prediction_hook->HookFunction(Functions::Prediction::IN_PREDICTION, 14);
    }
    if (state.surface_hook) {
        *(PDWORD)DataSection::ORIG_LOCK_CURSOR = state.surface_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkLockCursor, 67);
    }
    if (state.studio_render_hook) {
        *(PDWORD)DataSection::ORIG_DRAW_MODEL = state.studio_render_hook->HookFunction((DWORD)&Aimware2016Decompiled::VisualsEngine::hkDrawModel, 29);
    }

    *(PDWORD)DataSection::ORIG_ON_RENDER_START = (DWORD)DetourFunction((PBYTE)PatternScanner::FindOrZero("client.dll", "55 8B EC 83 EC ? 56 8B F1 57 89 75 ? E8 ? ? ? ? 8B CE E8"), (PBYTE)&Aimware2016Decompiled::ResolverEngine::hkOnRenderStart);

    // Netvar hooks - full reverse with decompiled proxies
    HookNetvar("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", DataSection::M_SMOKE_EFFECT_TICK_PROXY, Functions::Proxies::SMOKE_TICK);
    HookNetvar("CCSPlayer", "m_angEyeAngles[0]", 0, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnPitchProxy);
    HookNetvar("CCSPlayer", "m_angEyeAngles[1]", 0, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnYawProxy);
    HookNetvar("CCSPlayer", "m_flThirdpersonRecoil", DataSection::M_THIRDPERSON_RECOIL_PROXY, Functions::Proxies::RECOIL);
    HookNetvar("CCSPlayer", "m_flLowerBodyYawTarget", DataSection::M_LBY_PROXY, (uintptr_t)&Aimware2016Decompiled::ResolverEngine::OnLBYProxy);
    HookNetvar("CBasePlayer", "m_fFlags", DataSection::M_FLAGS_PROXY, Functions::Proxies::FLAGS);
    HookNetvar("CCSPlayer", "m_flFlashDuration", DataSection::M_FLASH_DURATION_PROXY, Functions::Proxies::FLASH);

#else
    LOG_INFO("Using ORIGINAL binary hooks (full reverse addresses)");

    if (state.engine_vgui_hook) *(PDWORD)DataSection::ORIG_PAINT = state.engine_vgui_hook->HookFunction(Functions::Paint::ADDRESS, 14);
    if (state.client_hook) {
        *(PDWORD)DataSection::ORIG_FRAME_STAGE = state.client_hook->HookFunction(Functions::FrameStageNotify::ADDRESS, 36);
        state.client_hook->HookFunction(Functions::Prediction::WRITE_USERCMD_DELTA, 23);
    }
    if (state.client_mode_hook) *(PDWORD)DataSection::ORIG_CREATE_MOVE = state.client_mode_hook->HookFunction(Functions::CreateMove::ADDRESS, 24);
    if (state.prediction_hook) {
        *(PDWORD)DataSection::ORIG_RUN_COMMAND = state.prediction_hook->HookFunction(Functions::Prediction::RUN_COMMAND, 19);
        *(PDWORD)DataSection::ORIG_SETUP_MOVE = state.prediction_hook->HookFunction(Functions::Prediction::SETUP_MOVE, 20);
        state.prediction_hook->HookFunction(Functions::Prediction::IN_PREDICTION, 14);
    }
    if (state.surface_hook) *(PDWORD)DataSection::ORIG_LOCK_CURSOR = state.surface_hook->HookFunction(Functions::LockCursor::ADDRESS, 67);
    if (state.studio_render_hook) *(PDWORD)DataSection::ORIG_DRAW_MODEL = state.studio_render_hook->HookFunction(Functions::DrawModel::ADDRESS, 29);

    *(PDWORD)DataSection::ORIG_ON_RENDER_START = (DWORD)DetourFunction((PBYTE)PatternScanner::FindOrZero("client.dll", "55 8B EC 83 EC ? 56 8B F1 57 89 75 ? E8 ? ? ? ? 8B CE E8"), (PBYTE)Functions::Proxies::ON_RENDER_START);

    HookNetvar("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", DataSection::M_SMOKE_EFFECT_TICK_PROXY, Functions::Proxies::SMOKE_TICK);
    HookNetvar("CCSPlayer", "m_angEyeAngles[0]", 0, Functions::Proxies::PITCH);
    HookNetvar("CCSPlayer", "m_angEyeAngles[1]", 0, Functions::Proxies::YAW);
    HookNetvar("CCSPlayer", "m_flThirdpersonRecoil", DataSection::M_THIRDPERSON_RECOIL_PROXY, Functions::Proxies::RECOIL);
    HookNetvar("CCSPlayer", "m_flLowerBodyYawTarget", DataSection::M_LBY_PROXY, Functions::Proxies::LBY);
    HookNetvar("CBasePlayer", "m_fFlags", DataSection::M_FLAGS_PROXY, Functions::Proxies::FLAGS);
    HookNetvar("CCSPlayer", "m_flFlashDuration", DataSection::M_FLASH_DURATION_PROXY, Functions::Proxies::FLASH);
#endif

    LOG_SUCCESS("Hooks initialized with full reverse");
    return true;
}

void Shutdown() {
    LOG_WARN("Shutting down Aimware with advanced mapper...");

    auto& state = GlobalState::Instance();
    state.Reset();
    
#ifdef USE_ADVANCED_MAPPER
    Mapping::AdvancedMapper::Instance().UnmapAll();
#else
    MemoryManager::Instance().FreeAll();
#endif

    LOG_INFO("Shutdown complete");
}

} // namespace Aimware

// ===================== Main Thread with Full Reverse =====================

DWORD WINAPI install_thread(PVOID) {
    Aimware::Logger::Instance().Initialize(true, false);
    LOG_INFO("=== Aimware 2016 Advanced Loader Started ===");
    LOG_INFO("Full Reverse Engineering Version");
    LOG_INFO("Bases: CODE=0x%08X DATA=0x%08X STRING=0x%08X CRT=0x%08X",
        Reversed::Bases::CODE_BASE, Reversed::Bases::DATA_BASE,
        Reversed::Bases::STRING_BASE, Reversed::Bases::CRT_BASE);

    // Wait for game
    LOG_INFO("Waiting for serverbrowser.dll...");
    while (!GetModuleHandleA("serverbrowser.dll")) {
        Sleep(100);
    }
    Sleep(500);

    LOG_INFO("Game modules loaded, analyzing dumps...");
    Aimware::AnalyzeDumps();

    LOG_INFO("Initializing memory with advanced mapper...");
    if (!Aimware::InitializeMemoryDumps()) {
        LOG_ERROR("Failed to initialize memory dumps");
        return 1;
    }

    auto& state = GlobalState::Instance();
    state.render = *(AwRender**)(DataSection::RENDER_CONTEXT_PTR);
    state.global_ctx = *(AwGlobals**)(Reversed::DataSection::GLOBAL_CONTEXT);
    state.skinchanger_ctx = *(AwSkinChangerData**)(DataSection::SKINCHANGER_CONTEXT);

    LOG_INFO("Fixing imports with full reverse...");
    Aimware::FixImportsAdvanced();

    LOG_INFO("Initializing interfaces with full reverse...");
    if (!Aimware::InitializeInterfaces()) {
        LOG_ERROR("Interface initialization failed");
    }

    LOG_INFO("Initializing netvars with full reverse...");
    if (!Aimware::InitializeNetvars()) {
        LOG_ERROR("Netvar initialization failed");
    }

    LOG_INFO("Fixing addresses with full reverse...");
    Aimware::FixAddresses();

    LOG_INFO("Fixing convars...");
    Aimware::FixConvars();

    LOG_INFO("Fixing post-OEP with full reverse...");
    Aimware::FixPostOEP();

    LOG_INFO("Initializing decompiled engine with full reverse...");
    Aimware2016Decompiled::InitializeDecompiledEngine();

    LOG_INFO("Initializing hooks with full reverse...");
    if (!Aimware::InitializeHooks()) {
        LOG_ERROR("Hook initialization failed");
        return 1;
    }

    state.initialized = true;
    LOG_SUCCESS("=== Aimware 2016 Loaded Successfully (Full Reverse) ===");
    LOG_INFO("Advanced Mapper: %s", state.useAdvancedMapper ? "ENABLED" : "DISABLED");
    LOG_INFO("Decrypted strings: %zu", state.decryptedStrings.size());
    LOG_INFO("Fixed regions: 0x34E10000 (CODE 192KB), 0x43AF0000 (DATA 90KB), 0x76ED0000 (STRING 110KB), 0x7C4A0000 (CRT 122KB)");
    LOG_INFO("XOR patches: %zu applied", xor_patches.size());
    LOG_INFO("Interfaces: %zu resolved", interfaces.size());
    LOG_INFO("Netvars: %zu tables", state.netvars.tables.size());
#ifdef USE_DECOMPILED_ENGINE
    LOG_INFO("Engine: DECOMPILED (full C++ reimplementation)");
#else
    LOG_INFO("Engine: BINARY (original 0x34E10000 code)");
#endif
    LOG_INFO("Press INSERT to open menu");

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
