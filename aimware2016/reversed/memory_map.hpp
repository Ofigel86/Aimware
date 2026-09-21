#pragma once
#include <cstdint>

/*
 * Full Reverse Engineering: Aimware 2016 Memory Map
 * 
 * This file documents ALL fixed addresses used by the binary dump.
 * Generated from deep analysis of b43AF0000.h (data section) and code references.
 */

namespace Aimware::Reversed {

// ===================== Fixed Base Addresses =====================
namespace Bases {
    constexpr uintptr_t CODE_BASE = 0x34E10000;
    constexpr size_t CODE_SIZE = 192512; // 0x2F000

    constexpr uintptr_t DATA_BASE = 0x43AF0000;
    constexpr size_t DATA_SIZE = 90112; // 0x16000

    constexpr uintptr_t STRING_BASE = 0x76ED0000;
    constexpr size_t STRING_SIZE = 110592; // 0x1B000

    constexpr uintptr_t CRT_BASE = 0x7C4A0000;
    constexpr size_t CRT_SIZE = 122880; // 0x1E000

    constexpr uintptr_t IMPORT_TABLE = 0x7C4B3E80;
    constexpr size_t IMPORT_TABLE_SIZE = 0xFC; // 252 bytes

    constexpr uintptr_t INTERFACE_TABLE = 0x43AFF000;
}

// ===================== Data Section Layout (0x43AF0000) =====================
namespace DataSection {

    // Lag records - allocated at runtime
    constexpr uintptr_t LAG_RECORDS_PTR = 0x43AF7A84; // Pointer to LagRecord[64]
    constexpr size_t LAG_RECORD_SIZE = 0x3234;
    constexpr size_t LAG_RECORD_COUNT = 64;

    // Profile context
    constexpr uintptr_t PROFILE_CONTEXT = 0x43AFF218; // profile_t*
    constexpr uintptr_t CONFIG_PATH = 0x43AF8B2C; // char[32] current config name

    // Render context
    constexpr uintptr_t RENDER_CONTEXT_PTR = 0x43B01224; // Actually 0x43AF7DAC
    constexpr uintptr_t RENDER_CONTEXT = 0x43AF7DAC; // AwRender*
    constexpr uintptr_t GLOBAL_CONTEXT = 0x43AF7704; // Actually 0x43AF7DC8
    constexpr uintptr_t SKINCHANGER_CONTEXT = 0x43AF7700; // Actually 0x43B01228

    // Interface pointers (0x43AFF000 table)
    constexpr uintptr_t VENGINE_CLIENT = 0x43AFF050; // VEngineClient013
    constexpr uintptr_t ENGINE_TRACE = 0x43AFF01C; // EngineTraceClient004
    constexpr uintptr_t MODEL_INFO = 0x43AFF04C; // VModelInfoClient004
    constexpr uintptr_t DEBUG_OVERLAY = 0x43AFF088; // VDebugOverlay004
    constexpr uintptr_t VCLIENT = 0x43AFF094; // VClient018
    constexpr uintptr_t ENTITY_LIST = 0x43AFEFDC; // VClientEntityList003
    constexpr uintptr_t PREDICTION = 0x43AFF0A8; // VClientPrediction001
    constexpr uintptr_t GAME_MOVEMENT = 0x43AFF0AC; // GameMovement001
    constexpr uintptr_t MATERIAL_SYSTEM = 0x43AFF000; // VMaterialSystem080
    constexpr uintptr_t SURFACE = 0x43AFF098; // VGUI_Surface031
    constexpr uintptr_t PHYSICS_PROPS = 0x43AFF028; // VPhysicsSurfaceProps001
    constexpr uintptr_t ENGINE_CVAR = 0x43AFF044; // VEngineCvar007
    constexpr uintptr_t MDL_CACHE = 0x43AFF0B0; // MDLCache
    constexpr uintptr_t LOCALIZE = 0x43AFF048; // Localize_

    // Random functions (vstdlib)
    constexpr uintptr_t RANDOM_FLOAT = 0x43AFF030;
    constexpr uintptr_t RANDOM_INT = 0x43AFF054;
    constexpr uintptr_t RANDOM_SEED = 0x43AFF03C;

    // Pattern-scanned functions
    constexpr uintptr_t WRITE_USERCMD = 0x43AFF0F0; // client.dll 55 8B EC 83 E4 F8...
    constexpr uintptr_t FINISH_DRAWING = 0x43AFF0F4; // vguimatsurface 8B 0D...
    constexpr uintptr_t START_DRAWING = 0x43AFF0F8; // vguimatsurface 55 8B EC...
    constexpr uintptr_t IS_BREAKABLE = 0x43AFF040; // client.dll 55 8B EC 51...
    constexpr uintptr_t CLIP_TRACE = 0x43AFF034; // client.dll 53 8B DC...
    constexpr uintptr_t TRACE_FILTER_SIMPLE = 0x43AFF0FC;
    constexpr uintptr_t TRACE_FILTER_SKIP_TWO = 0x43AFF100;

    // CSGO-specific
    constexpr uintptr_t GET_WEAPON_INFO = 0x43AFEF3C; // GetFileWeaponInfoFromHandle or GetCSWpnData
    constexpr uintptr_t FIND_HUD_ELEMENT = 0x43AFEFA0;
    constexpr uintptr_t HUD_PTR = 0x43AFEFA4;
    constexpr uintptr_t MD5_PSEUDORANDOM = 0x43AFEFD8;
    constexpr uintptr_t GLOW_MANAGER = 0x43AFEFC4;
    constexpr uintptr_t GLOBAL_VARS = 0x43AFF020; // CGlobalVarsBase
    constexpr uintptr_t CINPUT = 0x43AFF06C; // CInput
    constexpr uintptr_t MOVE_HELPER = 0x43AFEFE8;
    constexpr uintptr_t ITEM_SYSTEM = 0x43AFEFD0; // CCStrike15ItemSystem
    constexpr uintptr_t PREDICTION_RANDOM_SEED = 0x43AFEE5C;
    constexpr uintptr_t SMOKE_COUNT = 0x43AFF0B4;
    constexpr uintptr_t NAMESTEALER = 0x43AFF004;

    // CInput offsets (derived from CINPUT)
    constexpr uintptr_t CAMERA_IN_THIRD_PERSON = 0x43AFEF4C; // CINPUT + 0xA8
    constexpr uintptr_t VIEW_OFFSET = 0x43AFEF54; // CINPUT + 0xA5

    // Netvar proxies storage
    constexpr uintptr_t M_ANG_EYE_ANGLES_1_PROXY = 0x43B01318; // Actually RecvProp*
    constexpr uintptr_t M_SMOKE_TICK_PROXY = 0x43B01324;
    constexpr uintptr_t M_THIRDPERSON_RECOIL_PROXY = 0x43B01304;
    constexpr uintptr_t M_LBY_PROXY = 0x43B01334;
    constexpr uintptr_t M_FLAGS_PROXY = 0x43B01340;
    constexpr uintptr_t M_FLASH_DURATION_PROXY = 0x43B0134C;
    constexpr uintptr_t M_SMOKE_EFFECT_TICK_PROXY = 0x43B01328;

    // Class pointers
    constexpr uintptr_t BASE_WEAPON_WORLD_MODEL_CLASS = 0x43AFE0C4;
    constexpr uintptr_t BASE_VIEW_MODEL_CLASS = 0x43AFE0E0;

    // Client.dll base for internal pattern scanner
    constexpr uintptr_t CLIENT_BASE = 0x43AFF038; // base + 0x1000
    constexpr uintptr_t CLIENT_SIZE = 0x43AFF024;

    // WndProc
    constexpr uintptr_t ORIG_WNDPROC = 0x43AFF104;
    constexpr uintptr_t WINDOW_HANDLE = 0x43AFF214;

    // Unlock cursor flag
    constexpr uintptr_t UNLOCK_CURSOR_FLAG = 0x43AFE638;

    // Config flags (from disassembly)
    constexpr uintptr_t AIMBOT_ENABLED = 0x43AFBEA2;
    constexpr uintptr_t WEAPON_STATE = 0x43AFCD99;
    constexpr uintptr_t MENU_OPEN = 0x43AFD094;
    constexpr uintptr_t SKIN_ACTIVE = 0x43AFCCB2;

    // Command tracking
    constexpr uintptr_t COMMAND_NUMBER_ARRAY = 0x43AFE3D8; // int[150]?

    // Original function pointers (filled by hooks)
    constexpr uintptr_t ORIG_CREATE_MOVE = 0x43AFE178;
    constexpr uintptr_t ORIG_FRAME_STAGE = 0x43AFE630;
    constexpr uintptr_t ORIG_PAINT = 0x43AFE63C;
    constexpr uintptr_t ORIG_LOCK_CURSOR = 0x43AFE634;
    constexpr uintptr_t ORIG_DRAW_MODEL = 0x43AFE17C;
    constexpr uintptr_t ORIG_RUN_COMMAND = 0x43AFEE54;
    constexpr uintptr_t ORIG_SETUP_MOVE = 0x43AFE644;
    constexpr uintptr_t ORIG_ON_RENDER_START = 0x43AFE0E8;
}

// ===================== Code Section Functions (0x34E10000) =====================
namespace CodeSection {

    // Main hooks
    constexpr uintptr_t CREATE_MOVE = 0x34E258A0; // ClientMode::CreateMove
    constexpr uintptr_t FRAME_STAGE_NOTIFY = 0x34E26330; // CHLClient::FrameStageNotify
    constexpr uintptr_t PAINT = 0x34E26660; // EngineVGUI::Paint
    constexpr uintptr_t DRAW_MODEL = 0x34E25960; // StudioRender::DrawModel
    constexpr uintptr_t LOCK_CURSOR = 0x34E26620; // Surface::LockCursor
    constexpr uintptr_t WND_PROC = 0x34E33D60; // WndProc

    // Prediction
    constexpr uintptr_t RUN_COMMAND = 0x34E314B0; // Prediction::RunCommand
    constexpr uintptr_t SETUP_MOVE = 0x34E26880; // Prediction::SetupMove
    constexpr uintptr_t IN_PREDICTION = 0x34E26500;
    constexpr uintptr_t WRITE_USERCMD_DELTA = 0x34E268C0;

    // Rendering
    constexpr uintptr_t ESP_LOOP = 0x34E1D1D0; // Main ESP drawing
    constexpr uintptr_t CREATE_FONTS = 0x34E30FE4;

    // Skinchanger
    constexpr uintptr_t SKINCHANGER_UPDATE = 0x34E2C580; // FrameStageNotify skin update
    constexpr uintptr_t GLOVE_CHANGER = 0x34E1DFC9;
    constexpr uintptr_t PAINTKIT_CHANGER = 0x34E1E2DA;
    constexpr uintptr_t FILL_SKINS = 0x34E1F14E;

    // Netvar proxies
    constexpr uintptr_t PITCH_PROXY = 0x34E24BF0; // m_angEyeAngles[0]
    constexpr uintptr_t YAW_PROXY = 0x34E24D20; // m_angEyeAngles[1]
    constexpr uintptr_t LBY_PROXY = 0x34E24EC0; // m_flLowerBodyYawTarget
    constexpr uintptr_t FLASH_PROXY = 0x34E24B90;
    constexpr uintptr_t FLAGS_PROXY = 0x34E24F40;
    constexpr uintptr_t RECOIL_PROXY = 0x34E24FC0;
    constexpr uintptr_t SMOKE_PROXY = 0x34E24BC0;
    constexpr uintptr_t ON_RENDER_START = 0x34E25040; // CViewRender::OnRenderStart

    // Aimbot
    constexpr uintptr_t HITCHANCE = 0x34E282D0; // Seed calculation
    constexpr uintptr_t GET_BEST_TARGET = 0x34E28D40;
    constexpr uintptr_t AUTOWALL = 0x34E29040; // FireBullet
    constexpr uintptr_t MULTIPOINT = 0x34E29B90;

    // Antiaim
    constexpr uintptr_t PITCH_ANTIAIM = 0x34E2A910;
    constexpr uintptr_t YAW_ANTIAIM = 0x34E2AF90;
    constexpr uintptr_t FAKELAG = 0x34E2B720;

    // Visuals
    constexpr uintptr_t NO_SMOKE = 0x34E263A1; // Inside FrameStageNotify
    constexpr uintptr_t RADAR = 0x34E31D7B; // GetRadarHudElement

    // Config
    constexpr uintptr_t GEN_CONFIG_PATH = 0x34E34E90;
    constexpr uintptr_t DUMP_CONFIG = 0x34E357C5;
    constexpr uintptr_t GET_CONFIG_LIST = 0x34E35983;
    constexpr uintptr_t CONFIG_ACTION = 0x34E34746;
    constexpr uintptr_t PREPARE_CONFIG_SAVE = 0x34E1EAAD;

    // UI
    constexpr uintptr_t DRAW_MENU_HEADER = 0x34E37D73;
    constexpr uintptr_t DRAW_CHECKBOX = 0x34E372C5;
    constexpr uintptr_t DRAW_SLIDER = 0x34E37074;
    constexpr uintptr_t DRAW_KEYBIND = 0x34E38185;
    constexpr uintptr_t DRAW_COMBOBOX = 0x34E388A4;
    constexpr uintptr_t DRAW_TAB_BAR = 0x34E38FB2;
    constexpr uintptr_t DRAW_BUTTON = 0x34E39396;

    // Spam
    constexpr uintptr_t CHAT_SPAM = 0x34E2AD02;
    constexpr uintptr_t NAME_SPAM = 0x34E2AC29;

    // Init
    constexpr uintptr_t NETVAR_INIT = 0x34E1D890;
    constexpr uintptr_t CONFIG_LIST_REFRESH = 0x34E2B410;
    constexpr uintptr_t PANIC = 0x34E25530;
}

// ===================== String Section (0x76ED0000) =====================
namespace StringSection {
    constexpr uintptr_t GLOVE_MODEL_PATH = 0x76ED9CE0; // Encrypted glove model
    constexpr uintptr_t CHAMS_MATERIAL = 0x76ED9EA0; // Chams material override
    // ... many more encrypted strings
}

// ===================== Import Table (0x7C4B3E80) =====================
namespace ImportTable {
    // Kernel32
    constexpr uintptr_t GET_CURRENT_PROCESS_ID = 0x7C4B3E80;
    constexpr uintptr_t GET_FILE_SIZE = 0x7C4B3E84;
    constexpr uintptr_t FIND_FIRST_FILE_W = 0x7C4B3E88;
    constexpr uintptr_t FIND_CLOSE = 0x7C4B3E8C;
    constexpr uintptr_t FIND_NEXT_FILE_W = 0x7C4B3E90;
    constexpr uintptr_t GLOBAL_LOCK = 0x7C4B3E94;
    constexpr uintptr_t GLOBAL_ALLOC = 0x7C4B3E98;
    constexpr uintptr_t GLOBAL_UNLOCK = 0x7C4B3E9C;
    constexpr uintptr_t GLOBAL_FREE = 0x7C4B3EA0;
    constexpr uintptr_t MULTI_BYTE_TO_WIDE = 0x7C4B3EA4;

    // User32
    constexpr uintptr_t CLOSE_CLIPBOARD = 0x7C4B3EAC;
    constexpr uintptr_t IS_CLIPBOARD_AVAILABLE = 0x7C4B3EB0;
    constexpr uintptr_t GET_CLIPBOARD_DATA = 0x7C4B3EB4;
    constexpr uintptr_t GET_CURSOR_POS = 0x7C4B3EB8;
    constexpr uintptr_t CALL_WINDOW_PROC = 0x7C4B3EBC;
    constexpr uintptr_t GET_WINDOW_TEXT = 0x7C4B3EC0;
    constexpr uintptr_t SET_WINDOW_LONG = 0x7C4B3EC4;
    constexpr uintptr_t GET_RAW_INPUT = 0x7C4B3EC8;
    constexpr uintptr_t SCREEN_TO_CLIENT = 0x7C4B3ECC;
    constexpr uintptr_t GET_CLIENT_RECT = 0x7C4B3ED0;
    constexpr uintptr_t GET_WINDOW_THREAD_PID = 0x7C4B3ED4;
    constexpr uintptr_t SET_CLIPBOARD_DATA = 0x7C4B3EE0;
    constexpr uintptr_t OPEN_CLIPBOARD = 0x7C4B3EE4;
    constexpr uintptr_t EMPTY_CLIPBOARD = 0x7C4B3EE8;

    // MSVCRT
    constexpr uintptr_t TOLOWER = 0x7C4B3EF4;
    constexpr uintptr_t VSWPRINTF = 0x7C4B3EF8;
    constexpr uintptr_t WCSNCPY = 0x7C4B3EFC;
    constexpr uintptr_t STRNCPY = 0x7C4B3F00;
    constexpr uintptr_t MEMSET = 0x7C4B3F04;
    constexpr uintptr_t SSCANF = 0x7C4B3F08;
    constexpr uintptr_t SPRINTF = 0x7C4B3F0C;
    constexpr uintptr_t VSWPRINTF2 = 0x7C4B3F10;
    constexpr uintptr_t ATOI = 0x7C4B3F14;
    constexpr uintptr_t STRCHR = 0x7C4B3F18;
    constexpr uintptr_t STRSTR = 0x7C4B3F1C;
    constexpr uintptr_t CIFMOD = 0x7C4B3F20;
    constexpr uintptr_t ASINF = 0x7C4B3F24;
    constexpr uintptr_t ATAN = 0x7C4B3F28;
    constexpr uintptr_t ATAN2 = 0x7C4B3F2C;
    constexpr uintptr_t ATANF = 0x7C4B3F30;
    constexpr uintptr_t COSF = 0x7C4B3F34;
    constexpr uintptr_t POWF = 0x7C4B3F38;
    constexpr uintptr_t SINF = 0x7C4B3F3C;
    constexpr uintptr_t MEMCPY = 0x7C4B3F40;
    constexpr uintptr_t TOUPPER = 0x7C4B3F44;

    // NTDLL
    constexpr uintptr_t RTL_LEAVE_CRITICAL = 0x7C4B3F4C;
    constexpr uintptr_t RTL_ENTER_CRITICAL = 0x7C4B3F50;
    constexpr uintptr_t NT_QUERY_VIRTUAL_MEMORY = 0x7C4B3F54;
    constexpr uintptr_t NT_READ_FILE = 0x7C4B3F5C;
    constexpr uintptr_t NT_DELETE_FILE = 0x7C4B3F60;
    constexpr uintptr_t NT_CLOSE = 0x7C4B3F64;
    constexpr uintptr_t NT_CREATE_FILE = 0x7C4B3F68;
    constexpr uintptr_t RTL_INIT_UNICODE = 0x7C4B3F6C;
    constexpr uintptr_t NT_WRITE_FILE = 0x7C4B3F70;
    constexpr uintptr_t RTL_FREE_HEAP = 0x7C4B3F74;
    constexpr uintptr_t NT_DELAY_EXECUTION = 0x7C4B3F78;
    constexpr uintptr_t RTL_ALLOCATE_HEAP = 0x7C4B3F7C;
}

// ===================== XOR Patches =====================
namespace XorPatches {
    // These addresses contain JZ that should be JNZ (0x74 -> 0x75) to bypass XOR checks
    constexpr uintptr_t patches[] = {
        0x34E1EAAD, 0x34E1EB16, 0x34E1EB86, // Config Save
        0x34E1FE4A, 0x34E20169, 0x34E208A5, 0x34E20995, 0x34E20A8A, 0x34E20BC6, 0x34E20D55, 0x34E20E2F, // ESP
        0x34E1DFC9, 0x34E1E2DA, 0x34E1E32A, 0x34E1E38E, 0x34E1F14E, 0x34E1F190, 0x34E1F1D0, // SkinChanger
        0x34E25A13, // DrawModel Chams
        0x34E2AD02, 0x34E2AC29, // Spam
        0x34E37D73, 0x34E3791E, 0x34E38185, 0x34E3828B, 0x34E382D3, 0x34E388A4, 0x34E389AF, 0x34E38A52, // UI
        0x34E38FB2, 0x34E39396, 0x34E372C5, 0x34E37074, // UI
        0x34E36CC5, 0x34E36C45, 0x34E36BC5, 0x34E36B45, // Sliders
        0x34E35983, 0x34E357C5, 0x34E34746, // Config
        0x34E30DE4, 0x34E2B4BB, 0x34E31D7B, 0x34E30FE4, // Misc
        0x34E37EEF, 0x34E3837E, 0x34E38C48, 0x34E394BF, 0x34E39C5B, 0x34E3783F, 0x34E3774E, 0x34E36A6F, 0x34E3640E, // UI elements
        0x34E35028, 0x34E350C1, 0x34E34D58, 0x34E34C1A, 0x34E34C4C, 0x34E343AA, 0x34E34464, 0x34E2B347, 0x34E2B2EA, 0x34E2B473, // More
        0x34E239C7, 0x34E1F584, 0x34E1F26A, 0x34E1F2B5, 0x34E1F2F8, 0x34E1F002, 0x34E1F045, 0x34E1F088 // ESP
    };
    constexpr size_t PATCH_COUNT = sizeof(patches) / sizeof(patches[0]);
}

} // namespace Aimware::Reversed
