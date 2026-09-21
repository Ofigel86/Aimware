#include "aw.h"
#include "util.h"

#include "detours.h"
#include "hooking_manager.hpp"
#include "netvars_manager.hpp"

#define CSGO2016

bool call_in_bounds(void* addr)
{
	uintptr_t u = (uintptr_t)addr;
	return u > 0x34E10000 && u < (0x34E10000 + sizeof(b34E10000));
}

// reconstructed manually... guessed... ~95% accuracy
std::unordered_map<DWORD, std::pair<const char*, const char*>> imports = {
	// kernel32.dll, 100% restored
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

	// user32.dll, 90% restored
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

	// msvcrt.dll, 100% restored
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

	// ntdll.dll, 100% restored
	{ 0x7C4B3F4C, { "ntdll.dll", "RtlLeaveCriticalSection" } },
	{ 0x7C4B3F50, { "ntdll.dll", "RtlEnterCriticalSection" } },
	{ 0x7C4B3F54, { "ntdll.dll", "NtQueryVirtualMemory" } },
	//{ 0x7C4B3F58, { "ntdll.dll", "NtTerminateProcess" } }, // commented out for debugging reasons
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
	{ 0x43AFF0F0, { "client.dll", "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" } }, // WriteUserCmd
	{ 0x43AFF0F4, { "vguimatsurface.dll", "8B 0D ? ? ? ? 56 C6 05" } },// Surface__FinishDrawing
	{ 0x43AFF0F8, { "vguimatsurface.dll", "55 8B EC 83 E4 ? 83 EC ? 80 3D" } },// Surface__StartDrawing
	{ 0x43AFF040, { "client.dll", "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE" } },// IsBreakableEntity
#ifndef CSGO2018
	{ 0x43AFEF3C, { "client.dll", "66 3B 0D ? ? ? ? 72" } }, // GetFileWeaponInfoFromHandle
	{ 0x43AFEFA0, { "client.dll", "55 8B EC 51 8B C1 53 56 8B 75" } }, // FindHudElement
	{ 0x43AFEFD8, { "client.dll", "55 8B EC 83 E4 ? 83 EC ? 6A ? 8D 44 24" } }, // MD5_PseudoRandom
	{ 0x43AFEFC4, { "client.dll", "A1 ? ? ? ? A8 ? 75 ? 0F 57 C0 C7 05 ? ? ? ? 00 00 00 00 F3 0F 7F 05 ? ? ? ? 83 C8 ? C7 05 ? ? ? ? 00 00 00 00 66 0F 6F 05 ? ? ? ? 68 ? ? ? ? A3 ? ? ? ? F3 0F 7F 05 ? ? ? ? C7 05 ? ? ? ? 00 00 00 00 E8 ? ? ? ? 83 C4 ? B8" } }, // GetGlowObjectManager
#else
	{ 0x43AFEF3C, { "client.dll", "55 8B EC 81 EC ? ? ? ? 53 8B D9 56 57 8D 8B ? ? ? ? 85 C9 75 04 33 FF EB 2F" } }, // GetCSWpnData
	{ 0x43AFEFA0, { "client.dll", "55 8B EC 53 8B 5D ? 56 57 8B F9 33 F6 39 77 ? 7E ? 8B 47 ? ? ? ? ? ? FF 50" } }, // FindHudElement
	{ 0x43AFEFD8, { "client.dll", "55 8B EC 83 E4 ? 83 EC ? 6A ? 8D 44 24 ? 89 4C 24" } }, // MD5_PseudoRandom
	{ 0x43AFEFC4, { "client.dll", "A1 ? ? ? ? A8 01 75 4B" } }, // GetGlowObjectManager
#endif
	{ 0x43AFF034, { "client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10" } }, // ClipTraceToPlayers
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

	// commands
	{ 0x43AFF058, { "cl_fullupdate", true } },
};

std::vector<DWORD> xor_patches = {
	// PrepareConfigSave
	0x34E1EAAD, 0x34E1EB16, 0x34E1EB86,

	// DrawESP
	0x34E1FE4A, 0x34E20169, 0x34E208A5, 0x34E20995, 
	0x34E20A8A, 0x34E20BC6, 0x34E20D55, 0x34E20E2F, 

	0x34E1DFC9, // GloveChanger
	0x34E1E2DA, 0x34E1E32A, 0x34E1E38E, // PaintkitChanger

	0x34E1F14E, 0x34E1F190, 0x34E1F1D0, // FillSkins

	0x34E239C7, // SomethingEsp
	0x34E1F584, // SomethingEsp2
	0x34E1F26A, 0x34E1F2B5, 0x34E1F2F8, // SomethingEsp3
	0x34E1F002, 0x34E1F045, 0x34E1F088, // SomethingEsp4
	0x34E1F26A, 0x34E1F2B5, 0x34E1F2F8, // SomethingEsp5

	0x34E25A13, // hkDrawModel

	0x34E2AD02, // ChatSpam
	0x34E2AC29, // Namespam

	0x34E30DE4, // RenderWeapons

	0x34E2B4BB, // SomethingCfg

	0x34E31D7B, // GetRadarHudElement
	0x34E30FE4, // CreateFonts

	0x34E37D73, 0x34E3791E, // DrawMenuHeader

	0x34E38185, 0x34E3828B, 0x34E382D3, // DrawKeybind
	0x34E388A4, 0x34E389AF, 0x34E38A52, // DrawCombobox

	0x34E38FB2, // DrawTabBar
	0x34E39396, // DrawButton
	0x34E372C5, // DrawCheckbox
	0x34E37074, // DrawSlider
	0x34E36CC5, // SliderFmt
	0x34E36C45, // SliderFmt2
	0x34E36BC5, // SliderFmt3
	0x34E36B45, // SliderFmt4
	
	0x34E35983, // GetConfigList
	0x34E357C5, // DumpConfig
	0x34E34746, // ConfigAction

	// Something
	0x34E37EEF, // 0
	0x34E3837E, // 1
	0x34E38C48, // 2
	0x34E394BF, // 3
	0x34E39C5B, // 4
	0x34E3783F, // 5
	0x34E3774E, // 6
	0x34E36A6F, // 7
	0x34E3640E, // 8
	0x34E35028, 0x34E350C1, // 9
	0x34E34D58, // 10
	0x34E34C1A, 0x34E34C4C, // 11
	0x34E343AA, 0x34E34464, // 12
	0x34E2B347, // 13
	0x34E2B2EA, // 14

	0x34E2B473, // ConfigList parser
};

#define log(x) std::cout << x << std::endl;

AwRender* render = nullptr;
AwGlobals* global_ctx = nullptr;
AwSkinChangerData* skinchanger_ctx = nullptr;

void* engine_vgui = nullptr;
vmthook* engine_vgui_hook = nullptr;

IBaseClientDLL* client = nullptr;
vmthook* client_hook = nullptr;

void* client_mode = nullptr;
vmthook* client_mode_hook = nullptr;

void* prediction = nullptr;
vmthook* prediction_hook = nullptr;

void* surface = nullptr;
vmthook* surface_hook = nullptr;

void* trace = nullptr;
vmthook* trace_hook = nullptr;

void* studio_render = nullptr;
vmthook* studio_render_hook = nullptr;

void* view_render = nullptr;
vmthook* view_render_hook = nullptr;

void* fire_bullets = nullptr;
vmthook* fire_bullets_hook = nullptr;

ICvar* cvars = nullptr;

HWND window; 
WNDPROC orig_wndproc;

netvars _netvars;

std::vector<std::pair<uintptr_t, uintptr_t>> hooked_netvars;

void hook_netvar(const char* table, const char* var, uintptr_t original_addr, uintptr_t hook_fn)
{
	RecvProp* prop = nullptr;
	_netvars.get_prop(table, var, &prop);

	if (!prop)
	{
		log("netvar " << table << "::" << var << " not found");
		return;
	}

	hooked_netvars.push_back({ (uintptr_t)prop->proxy, (uintptr_t)prop });

	if (original_addr)
		*(void**)original_addr = prop->proxy;
	prop->proxy = (recvProxy)hook_fn;
}

void unhook_netvars()
{
	for (auto& netvar : hooked_netvars)
	{
		((RecvProp*)netvar.second)->proxy = (recvProxy)netvar.first;
	}

	hooked_netvars.clear();
}

void init_aw_ptrs()
{
	render = *(AwRender**)(0x43B01224); // actual addr is 43AF7DAC btw
	global_ctx = *(AwGlobals**)(0x43AF7704); // actual addr is 43AF7DC8 btw
	skinchanger_ctx = *(AwSkinChangerData**)(0x43AF7700); // actual addr is 43B01228 btw
}

void fix_imports()
{
	for (auto& import : imports)
	{
		HMODULE module = LoadLibraryA(import.second.first);
		if (!module)
		{
			log("module " << import.second.first << " not found");
			continue;
		}

		FARPROC procedure = GetProcAddress(module, import.second.second);
		if (!procedure)
		{
			log("import " << import.second.first << "::" << import.second.second << " not found");
			continue;
		}

		*(FARPROC*)import.first = procedure;
	}
}

void fix_addresses()
{
	for (auto& pattern : patterns)
	{
		uintptr_t pat = find_signature(pattern.second.first, pattern.second.second);
		if (!pat)
		{
			log("pattern \"" << pattern.second.second << "\" not found");
			continue;
		}

		*(uintptr_t*)pattern.first = pat;
	}

	// pHud for FindElement
	*(PDWORD)0x43AFEFA4 = *(PDWORD)(find_signature("client.dll", "B9 ? ? ? ? 56 68 ? ? ? ? 89 45") + 1);

	// PredictionRandomSeed
	*(PDWORD)0x43AFEE5C = *(DWORD*)(find_signature("client.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);

	// SmokeCount
	*(PDWORD)0x43AFF0B4 = *(DWORD*)(find_signature("client.dll", "A3 ? ? ? ? 57 8B CB") + 1);

	// Namestealer, namespam crap
	*(PDWORD)0x43AFF004 = *(DWORD*)(find_signature("engine.dll", "38 05 ? ? ? ? 75 ? 8B CE C6 05 ? ? ? ? ? E8 ? ? ? ? C6 05 ? ? ? ? 00") + 2);

	// CInput m_bCameraInThirdPerson and m_vecViewOffset references. Thank aw devs for that... Is it really that difficult to get them directly from Input?
	*(PDWORD)0x43AFEF4C = ((*(PDWORD)0x43AFF06C) + 0xA8);
	*(PDWORD)0x43AFEF54 = ((*(PDWORD)0x43AFF06C) + 0xA5);

	// Get rid of stupid TraceRay and ClipTraceToPlayers hooks...
	*(PDWORD)0x43AFF0FC = *(DWORD*)(find_signature("client.dll", "C7 45 ? ? ? ? ? C7 45 ? 00 00 00 00 FF 50 ? A1") + 3); // CTraceFilterSimple_vtable
	*(PDWORD)0x43AFF100 = *(DWORD*)(find_signature("client.dll", "C7 44 24 ? ? ? ? ? FF 90 ? ? ? ? 8D 44 24 ? 50 8D 44 24 ? 50 68 ? ? ? ? 8B 55") + 4); // CTraceFilterSkipTwoEntities_vtable
}

void fix_cvars()
{
	cvars = get_interface<ICvar>("vstdlib.dll", "VEngineCvar");
	for (auto cvar : convars)
	{
		void* cvar_ptr = cvar.second.second ? (void*)cvars->FindCommand(cvar.second.first) : (void*)cvars->FindVar(cvar.second.first);
		if (!cvar_ptr)
		{
			log("cvar \"" << cvar.second.first << "\" not found");
			continue;
		}

		*(void**)cvar.first = cvar_ptr;
	}
}

void init_interfaces()
{
	for (auto& interface_ : interfaces)
	{
		void* interface_ptr = strstr(interface_.second.second, "Random") ? GetProcAddress(GetModuleHandleA(interface_.second.first), interface_.second.second) : get_interface<void>(interface_.second.first, interface_.second.second);
		if (!interface_ptr)
		{
			log("interface \"" << interface_.second.second << "\" not found");
			continue;
		}

		*(void**)interface_.first = interface_ptr;
	}

	client = *(IBaseClientDLL**)(0x43AFF094);
	client_mode = **(void***)((*(DWORD**)client)[10] + 0x5);
	prediction = *(void**)(0x43AFF0A8);
	surface = *(void**)(0x43AFF098);
	trace = *(void**)(0x43AFF01C);

#ifndef CSGO2018
	*(PDWORD)0x43AFF020 = **(DWORD**)((*(DWORD**)(client))[0] + 0x53); // CGlobalVarsBase
#else
	*(PDWORD)0x43AFF020 = **(DWORD**)((*(DWORD**)(client))[0] + 0x1B); // CGlobalVarsBase
#endif
	*(PDWORD)0x43AFF06C = *reinterpret_cast<DWORD*>((*reinterpret_cast<uintptr_t**>(client))[15] + 0x1); // CInput
	*(PDWORD)0x43AFEFE8 = **(DWORD**)(find_signature("client.dll", "8B 0D ? ? ? ? 8B 46 08 68") + 0x2); // MoveHelper

	*(PDWORD)0x43AFEFD0 = (DWORD)reinterpret_cast<CCStrike15ItemSystem*(*)()>(find_signature("client.dll", "A1 ? ? ? ? 85 C0 75 ? A1 ? ? ? ? 56 68"))(); // ItemSystem
}

#ifdef CSGO2018
void fix_for_2018()
{
	// skinchanger fix
	*(PDWORD)0x34E1D948 += 0x24; // 0xF0
	*(PDWORD)0x34E1D954 += 0x24; // 0x284
	*(PDWORD)0x34E1D960 += 0x24; // 0x218

	*(PDWORD)0x34E1D989 += 0x24; // 0xF0
	*(PDWORD)0x34E1DA1D += 0x24; // 0xF0

	*(PDWORD)0x34E1D9A2 += 0x24; // 0xD8

	*(PDWORD)0x34E1DB2F += 0x24; // 0x218
	*(PDWORD)0x34E1DB52 += 0x24; // 0x200
	*(PDWORD)0x34E1DBEA += 0x24; // 0x218

	*(PDWORD)0x34E1DC3A += 0x24; // 0x26C
	*(PDWORD)0x34E1DC1B += 0x24; // 0x284

	*(PDWORD)0x34E31F0B += 0x24; // 0xF0
	*(PDWORD)0x34E31F15 += 0x24; // 0xD8

	// getspread, etc indexes
	*(PDWORD)0x34E1C021 = 469 * 4;
	*(PDWORD)0x34E1C015 = 471 * 4;
	*(PDWORD)0x34E1BFD6 = 439 * 4;

	// patch for GetCSWpnData
	*(PBYTE)0x34E1C038 = 0x89;
	*(PBYTE)0x34E1C039 = 0xF9;
	*(PBYTE)0x34E1C03A = 0x90;
	*(PBYTE)0x34E1C03B = 0x90;

	*(PSHORT)0x34E1C046 = 0x00C8;
	*(PSHORT)0x34E1C052 = 0x00F0;
	*(PSHORT)0x34E1C05E = 0x00F8;
	*(PSHORT)0x34E1C06A = 0x00EC;
	*(PSHORT)0x34E1C076 = 0x0104;
	*(PSHORT)0x34E1C082 = 0x0108;
	*(PSHORT)0x34E1C08E = 0x00F4;
	*(PSHORT)0x34E1C09A = 0x0000;

	// nop out some crap
	for (PBYTE byte = (PBYTE)0x34E318C6; (DWORD)byte < 0x34E318D7; byte++)
		*byte = 0x90;

	for (PBYTE byte = (PBYTE)0x34E318DA; (DWORD)byte < 0x34E318DF; byte++)
		*byte = 0x90;

	for (PBYTE byte = (PBYTE)0x34E318E4; (DWORD)byte < 0x34E31918; byte++)
		*byte = 0x90;

	// fix netvars
	*(PDWORD)0x43AFEF34 = 0x29BC;
	*(PDWORD)0x43AFEFC8 = 0xA310;
	*(PDWORD)0x43AFEFB8 = 0x32B0;
	*(PDWORD)0x43AFEED8 = 0x3360;
	*(PDWORD)0x43AFEEE0 = _netvars.get_offset("CCSPlayer", "deadflag") + 4;
	*(PDWORD)0x43AFEEE4 = _netvars.get_offset("CCSPlayer", "m_nTickBase");

	*(PDWORD)0x43AFEEA0 = _netvars.get_offset("CCSPlayer", "m_flNextAttack");
	*(PDWORD)0x43AFEEB4 = _netvars.get_offset("CCSPlayer", "m_flPoseParameter");
	*(PDWORD)0x43AFEEBC = _netvars.get_offset("CCSPlayer", "m_bClientSideAnimation");
	*(PDWORD)0x43AFEF70 = _netvars.get_offset("CCSPlayer", "m_bHasHelmet");
	*(PDWORD)0x43AFEF74 = _netvars.get_offset("CCSPlayer", "m_bHasDefuser");
	*(PDWORD)0x43AFEF78 = _netvars.get_offset("CCSPlayer", "m_iAccount");
	*(PDWORD)0x43AFEF7C = _netvars.get_offset("CCSPlayer", "m_bIsDefusing");
	*(PDWORD)0x43AFEF80 = _netvars.get_offset("CCSPlayer", "m_flLowerBodyYawTarget");
	*(PDWORD)0x43AFEF84 = _netvars.get_offset("CCSPlayer", "m_iShotsFired");
	*(PDWORD)0x43AFEF88 = _netvars.get_offset("CCSPlayer", "m_bGunGameImmunity");
	*(PDWORD)0x43AFEE9C = _netvars.get_offset("CCSPlayer", "m_aimPunchAngle");

	*(PDWORD)0x43AFEF6C = _netvars.get_offset("CBasePlayer", "m_ArmorValue");

	*(PDWORD)0x43AFEEA8 = _netvars.get_offset("CBaseCombatCharacter", "m_hActiveWeapon");
	*(PDWORD)0x43AFEEAC = _netvars.get_offset("CBaseCombatCharacter", "m_hMyWearables");

	*(PDWORD)0x43AFEEF4 = 0x31D8;
	*(PDWORD)0x43AFEEF8 = _netvars.get_offset("CBaseCombatWeapon", "m_iItemDefinitionIndex");
	*(PDWORD)0x43AFEEFC = _netvars.get_offset("CBaseCombatWeapon", "m_iClip1");
	*(PDWORD)0x43AFEF38 = _netvars.get_offset("CBaseCombatWeapon", "m_iClip2");
	*(PDWORD)0x43AFEF40 = _netvars.get_offset("CBaseCombatWeapon", "m_iAccountID");
	*(PDWORD)0x43AFEF20 = _netvars.get_offset("CBaseCombatWeapon", "m_iViewModelIndex");
	*(PDWORD)0x43AFEF24 = _netvars.get_offset("CBaseCombatWeapon", "m_iWorldModelIndex");
	*(PDWORD)0x43AFEF30 = _netvars.get_offset("CBaseCombatWeapon", "m_iPrimaryReserveAmmoCount");
	*(PDWORD)0x43AFEFB4 = _netvars.get_offset("CBaseCombatWeapon", "m_flPostponeFireReadyTime");

	*(PDWORD)0x43AFEF00 = _netvars.get_offset("CBaseAttributableItem", "m_nFallbackStatTrak");
	*(PDWORD)0x43AFEF04 = _netvars.get_offset("CBaseAttributableItem", "m_nFallbackPaintKit");
	*(PDWORD)0x43AFEF08 = _netvars.get_offset("CBaseAttributableItem", "m_OriginalOwnerXuidLow");
	*(PDWORD)0x43AFEF0C = _netvars.get_offset("CBaseAttributableItem", "m_bInitialized");
	*(PDWORD)0x43AFEF10 = _netvars.get_offset("CBaseAttributableItem", "m_szCustomName");
	*(PDWORD)0x43AFEF14 = _netvars.get_offset("CBaseAttributableItem", "m_iItemIDLow");
	*(PDWORD)0x43AFEF28 = _netvars.get_offset("CBaseAttributableItem", "m_nFallbackSeed");
	*(PDWORD)0x43AFEF2C = _netvars.get_offset("CBaseAttributableItem", "m_flFallbackWear");

	// pHud for FindElement
	*(PDWORD)0x43AFEFA4 = *(PDWORD)(find_signature("client.dll", "B9 ? ? ? ? 0F 94 C0 0F B6 C0 50 68") + 1);

	// PredictionRandomSeed
	*(PDWORD)0x43AFEE5C = *(DWORD*)(find_signature("client.dll", "C7 05 ? ? ? ? ? ? ? ? EB ? 8B 47") + 2);
}
#endif

void fix_post_oep_crap()
{
	log("fixing render... (original res: " << render->Width << "x" << render->Height << ")")

	render->DidCreateFont = false;
	render->Width = 0;
	render->Height = 0;

	log("fixing xor...");
	for (auto& xor_addr : xor_patches)
	{
		*(PBYTE)xor_addr = 0x75;
	}

	while (!(window = FindWindowA("Valve001", nullptr)))
		Sleep(100);

	// reset skinchanger struct so it does not create weird issues inside skin changer window
	std::memset(skinchanger_ctx, 0, 216);

	*(PDWORD)0x43B01318 = (DWORD)_netvars.get_prop("CCSPlayer", "m_angEyeAngles[1]"); // m_angEyeAngles[1]
	*(PDWORD)0x43B01324 = (DWORD)_netvars.get_prop("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin"); // m_nSmokeEffectTickBegin

	*(PDWORD)0x43AFE0C4 = (DWORD)_netvars.get_class("CBaseWeaponWorldModel"); // CBaseWeaponWorldModel
	*(PDWORD)0x43AFE0E0 = (DWORD)_netvars.get_class("CBaseViewModel"); // CBaseViewModel

	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("client.dll"), &info, 12);

	*(PDWORD)0x43AFF038 = (DWORD)info.lpBaseOfDll + 0x1000; // client.dll base for pattern scanner
	*(PDWORD)0x43AFF024 = info.SizeOfImage; // client.dll size for pattern scanner

	char* records_mem = new char[0x3234 * 64];
	*(PDWORD)0x43AF7A84 = (DWORD)records_mem; // allocate lagrecords
	std::memset(records_mem, 0, 0x3234 * 64);

	*(PDWORD)0x43AFE638 = 0; // UnlockCursor?

	// *(PDWORD)0x43AFF004 = (DWORD)new int; // unknown var, patch rn (Namestealer, Namespam)
	// *(PCHAR)0x43AFCCB2 = 0; // skin_active cfg

	struct profile_t
	{
		int xor_key = 0;
		int pad;
		wchar_t config_path[260];
		int pad2[64];
	};

	profile_t* profile_ = new profile_t;
	std::memset(profile_, 0, sizeof(profile_t));

	profile_->xor_key = 0;
	std::memcpy(profile_->config_path, L"\\??\\C:\\aimware\\", sizeof(L"\\??\\C:\\aimware\\"));

	*(PDWORD)0x43AFF218 = (DWORD)profile_;

	CreateDirectoryA("C:\\aimware\\", 0);

	for (PBYTE byte = (PBYTE)0x34E1DA71; (DWORD)byte < 0x34E1DA82; byte++)
		*byte = 0x90;

	for (PBYTE byte = (PBYTE)0x34E1DA85; (DWORD)byte < 0x34E1DA8A; byte++)
		*byte = 0x90;

	for (PBYTE byte = (PBYTE)0x34E1DA8F; (DWORD)byte < 0x34E1DA94; byte++)
		*byte = 0x90;

	for (PBYTE byte = (PBYTE)0x34E1DA9A; (DWORD)byte < 0x34E1DA9D; byte++)
		*byte = 0x90;

	for (PBYTE byte = (PBYTE)0x34E1DAA2; (DWORD)byte < 0x34E1DAA5; byte++)
		*byte = 0x90;

	for (PBYTE byte = (PBYTE)0x34E1DAA8; (DWORD)byte < 0x34E1DAAD; byte++)
		*byte = 0x90;

#ifdef CSGO2018
	fix_for_2018();
#endif

	// call netvar & skins init.
	((void(*)())(0x34E1D890))();

	// refresh config list
	((void(*)())(0x34E2B410))();

	// reset config name
	std::memset((void*)0x43AF8B2C, 0, 32);
}

void __cdecl hkPanic()
{
	engine_vgui_hook->unhook();
	client_hook->unhook();
	client_mode_hook->unhook();
	prediction_hook->unhook();
	surface_hook->unhook();
	trace_hook->unhook();
	studio_render_hook->unhook();

	unhook_netvars();

	((RecvProp*)skinchanger_ctx->sequence_prop)->proxy = (recvProxy)skinchanger_ctx->sequence_proxy;

	SetWindowLongPtr(window, GWL_WNDPROC, (LONG)orig_wndproc);
	return;
}

void __stdcall hkGenConfigPath(const char* name, wchar_t* out)
{
	std::memset(out, 0, sizeof(wchar_t) * 64);

	int pos = strlen(name);
	for (int i = 0; i < pos; i++)
		out[i] = (wchar_t)name[i];
}

void init_aw_hooks()
{ 
	// 34E26660 - hkEngineVGUI_Paint | 34E33D60 - hkWndProc

	DetourFunction((PBYTE)0x34E34E90, (PBYTE)hkGenConfigPath); // visual-only. make cfg filenames readable
	DetourFunction((PBYTE)0x34E25530, (PBYTE)hkPanic);

	orig_wndproc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)((void*)0x34E33D60));

	*(WNDPROC*)(0x43AFF104) = orig_wndproc;
	*(HWND*)(0x43AFF214) = window;

	*(PDWORD)0x43AFE63C = engine_vgui_hook->hook_function(0x34E26660, 14); // EngineVGUI::Paint original
	*(PDWORD)0x43AFE630 = client_hook->hook_function(0x34E26330, 36); // CHLClient::FrameStageNotify original
	*(PDWORD)0x43AFE178 = client_mode_hook->hook_function(0x34E258A0, 24); // ClientMode::CreateMove original
	*(PDWORD)0x43AFEE54 = prediction_hook->hook_function(0x34E314B0, 19); // Prediction::RunCommand original
	*(PDWORD)0x43AFE644 = prediction_hook->hook_function(0x34E26880, 20); // Prediction::SetupMove original

	*(PDWORD)0x43AFE634 = surface_hook->hook_function(0x34E26620, 67); // Surface::LockCursor original
	*(PDWORD)0x43AFE17C = studio_render_hook->hook_function(0x34E25960, 29); // StudioRender::DrawModel original

	client_hook->hook_function(0x34E268C0, 23); // CHLClient::WriteUserCmdDeltaToBuffer, no original needed
	prediction_hook->hook_function(0x34E26500, 14); // Prediction::InPrediction, no original needed

	*(PDWORD)0x43AFE0E8 = (DWORD)DetourFunction((PBYTE)find_signature("client.dll", "55 8B EC 83 EC ? 56 8B F1 57 89 75 ? E8 ? ? ? ? 8B CE E8"), (PBYTE)0x34E25040); // CViewRender::OnRenderStart original

	/////////////////////////////////////////////////////

	hook_netvar("CSmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", 0x43B01328, 0x34E24BC0);

	hook_netvar("CCSPlayer", "m_angEyeAngles[0]", 0, 0x34E24BF0);
	hook_netvar("CCSPlayer", "m_angEyeAngles[1]", 0, 0x34E24D20);
	hook_netvar("CCSPlayer", "m_flThirdpersonRecoil", 0x43B01304, 0x34E24FC0);
	hook_netvar("CCSPlayer", "m_flLowerBodyYawTarget", 0x43B01334, 0x34E24EC0);
	hook_netvar("CBasePlayer", "m_fFlags", 0x43B01340, 0x34E24F40);
	hook_netvar("CCSPlayer", "m_flFlashDuration", 0x43B0134C, 0x34E24B90);
}

void init_local_interfaces()
{
	engine_vgui = get_interface<void>("engine.dll", "VEngineVGui0");
	engine_vgui_hook = new vmthook(reinterpret_cast<DWORD**>(engine_vgui));

	studio_render = get_interface<void>("studiorender.dll", "VStudioRender");
	studio_render_hook = new vmthook(reinterpret_cast<DWORD**>(studio_render));

	// who cares? data retrieved from Firebullets::PostDataUpdate hook doesnt seem to be used anywhere
	//fire_bullets = *(void**)(find_signature("client.dll", "8B D1 B8 ? ? ? ? 51") + 0x91);
	//fire_bullets_hook = new vmthook(reinterpret_cast<DWORD**>(fire_bullets));

	client_hook = new vmthook(reinterpret_cast<DWORD**>(client));
	client_mode_hook = new vmthook(reinterpret_cast<DWORD**>(client_mode));
	prediction_hook = new vmthook(reinterpret_cast<DWORD**>(prediction));
	surface_hook = new vmthook(reinterpret_cast<DWORD**>(surface));
	trace_hook = new vmthook(reinterpret_cast<DWORD**>(trace));
}

void init_netvars()
{
	_netvars.tables.clear();

	auto clientclass = client->GetAllClasses();
	if (!clientclass)
		return;

	while (clientclass)
	{
		auto recvTable = clientclass->m_pRecvTable;
		if (recvTable)
		{
			_netvars.classes.emplace(std::string(clientclass->m_pNetworkName), clientclass);
			_netvars.tables.emplace(std::string(clientclass->m_pNetworkName), recvTable);
		}

		clientclass = clientclass->m_pNext;
	}
}

DWORD WINAPI install_thread(PVOID a1)
{
	AllocConsole();

	FILE* dum;
	freopen_s(&dum, "CONOUT$", "w", stdout);

	while (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(1);

	log("copying dumps...");
	std::memcpy((void*)0x7C4A0000, b7C4A0000, sizeof(b7C4A0000));
	std::memcpy((void*)0x76ED0000, b76ED0000, sizeof(b76ED0000));

	std::memcpy((void*)0x43AF0000, b43AF0000, sizeof(b43AF0000));
	std::memcpy((void*)0x34E10000, b34E10000, sizeof(b34E10000));

	log("getting aimware structs...");
	init_aw_ptrs();

	log("fixing imports...");
	fix_imports();

	log("initializing interfaces...");
	init_interfaces();
	init_local_interfaces();

	log("initializing netvars...");
	init_netvars();

	log("fixing addresses...");
	fix_addresses();

	log("fixing cvars...");
	fix_cvars();

	log("restoring stuff...");
	fix_post_oep_crap();

	log("initializing hooks...");
	init_aw_hooks();

	log("done!");
	return EXIT_SUCCESS;
}

BOOL WINAPI DllMain(void* a1, int reason, void* a2)
{
	if (reason == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, install_thread, 0, 0, 0);

	return TRUE;
}