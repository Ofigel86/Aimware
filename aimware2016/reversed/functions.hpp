#pragma once
#include <cstdint>

/*
 * Full Reverse Engineering: Aimware 2016 Function Signatures
 * 
 * This file documents all important functions from code section 0x34E10000
 * with their disassembly analysis and C++ reconstruction.
 */

namespace Aimware::Reversed::Functions {

// ===================== Hook Handlers =====================

// ClientMode::CreateMove - Main aimbot entry
// VA: 0x34E258A0
// Signature: bool __stdcall hkCreateMove(float input_sample_frametime, CUserCmd* cmd)
// Disassembly:
// 34e258a0: push ebp
// 34e258a1: mov ebp, esp
// 34e258a4: cmp byte ptr [0x43afbea2], 0   ; Aimbot enabled?
// 34e258ab: je 0x34e258b6
// 34e258ad: cmp byte ptr [0x43afcd99], 2   ; Weapon state?
// 34e258b4: jne 0x34e258ca
// 34e258b6: push [ebp+0xC]                 ; CUserCmd*
// 34e258b9: movss xmm0, [ebp+0x8]          ; frametime
// 34e258c4: call dword ptr [0x43afe178]    ; Original CreateMove
// ...
// 34e2590c: mov [0x43afe3d8 + edx*4], eax  ; Store command number
// 34e25918: call 0x34e282d0                ; Hitchance & seed calc
namespace CreateMove {
    constexpr uintptr_t ADDRESS = 0x34E258A0;
    constexpr uintptr_t ORIGINAL_PTR = 0x43AFE178;
    constexpr uintptr_t ENABLED_FLAG = 0x43AFBEA2;
    constexpr uintptr_t WEAPON_STATE = 0x43AFCD99;
    constexpr uintptr_t COMMAND_NUM_ARRAY = 0x43AFE3D8;
    constexpr uintptr_t HITCHANCE_CALL = 0x34E282D0;

    // Pseudo-C++:
    /*
    bool __stdcall hkCreateMove(float frametime, CUserCmd* cmd) {
        if (!cmd || !cmd->command_number) return false;
        if (!*(bool*)0x43AFBEA2) { // aimbot enabled?
            return original(frametime, cmd);
        }
        // ... aimbot logic
        int bestTarget = GetBestTarget(cmd, targetPos);
        if (bestTarget != -1) {
            QAngle aim = CalcAngle(localEye, targetPos);
            if (CalculateHitchance(cmd, aim, hitchance)) {
                cmd->viewangles = aim;
                if (autoShoot) cmd->buttons |= IN_ATTACK;
            }
        }
        return false; // silent aim
    }
    */
}

// CHLClient::FrameStageNotify
// VA: 0x34E26330
// Signature: void __thiscall hkFrameStageNotify(ClientFrameStage_t stage)
// Disassembly:
// 34e26330: push ebp
// 34e26331: mov ebp, esp
// 34e2633c: mov ecx, [0x43aff050]          ; VEngineClient
// 34e26344: mov eax, [eax+0x30]            ; IsInGame()
// 34e26347: call eax
// ...
// 34e263a1: mov eax, [0x43aff0b4]          ; Smoke count
// 34e263a6: mov dword ptr [eax], 0         ; NoSmoke!
// 34e263f3: cmp ebx, 4                     ; FRAME_NET_UPDATE_POSTDATAUPDATE_START
// 34e263fc: call 0x34E2C580                ; SkinChanger
namespace FrameStageNotify {
    constexpr uintptr_t ADDRESS = 0x34E26330;
    constexpr uintptr_t ORIGINAL_PTR = 0x43AFE630;
    constexpr uintptr_t ENGINE_CLIENT = 0x43AFF050;
    constexpr uintptr_t ENTITY_LIST = 0x43AFEFDC;
    constexpr uintptr_t SMOKE_COUNT = 0x43AFF0B4;
    constexpr uintptr_t SKINCHANGER_CALL = 0x34E2C580;
    constexpr int POSTDATAUPDATE_START = 4;

    /*
    void __thiscall hkFrameStageNotify(void* ecx, int stage) {
        original(ecx, stage);
        if (!engine->IsInGame()) return;
        
        if (noSmoke) {
            *smokeCount = 0; // 0x43AFF0B4
        }
        
        if (stage == 4) { // FRAME_NET_UPDATE_POSTDATAUPDATE_START
            SkinChangerUpdate(); // 0x34E2C580
        }
        
        // Thirdperson, etc
    }
    */
}

// EngineVGUI::Paint
// VA: 0x34E26660
// Signature: void __thiscall hkPaint(void* ecx, int mode)
// Disassembly:
// 34e26660: push ebx
// 34e26661: mov ebx, [esp+0x8]             ; mode
// 34e26666: call dword ptr [0x43afe63c]    ; Original Paint
// 34e2666c: test bl, 1                     ; PAINT_UIPANELS?
// 34e2666f: je 0x34e267c3
// 34e26675: mov ecx, [0x43af7704]          ; Global context
// 34e2667b: call 0x34e1d1d0                ; ESP & GUI loop
namespace Paint {
    constexpr uintptr_t ADDRESS = 0x34E26660;
    constexpr uintptr_t ORIGINAL_PTR = 0x43AFE63C;
    constexpr uintptr_t GLOBAL_CONTEXT = 0x43AF7704;
    constexpr uintptr_t ESP_CALL = 0x34E1D1D0;
    constexpr int PAINT_UIPANELS = 1;

    /*
    void __thiscall hkPaint(void* ecx, int mode) {
        original(ecx, mode);
        if (mode & 1) { // PAINT_UIPANELS
            DrawESP(); // 0x34E1D1D0
        }
    }
    */
}

// StudioRender::DrawModel (Chams)
// VA: 0x34E25960
// Signature: void __thiscall hkDrawModel(void* ecx, DrawModelResults_t* results, ModelRenderInfo_t* info, matrix3x4_t* boneToWorld, ...)
namespace DrawModel {
    constexpr uintptr_t ADDRESS = 0x34E25960;
    constexpr uintptr_t ORIGINAL_PTR = 0x43AFE17C;
    constexpr uintptr_t SKIN_ACTIVE = 0x43AFCCB2;
    constexpr uintptr_t PROFILE_CONTEXT = 0x43AFF218;
    constexpr uintptr_t CHAMS_MATERIAL_STRING = 0x76ED9EA0;

    /*
    void __thiscall hkDrawModel(...) {
        if (!*(bool*)0x43AFCCB2) { // skin_active / chams enabled?
            return original(...);
        }
        // Get local player
        // Get profile XOR key
        // Decrypt chams material string at 0x76ED9EA0
        // Apply material override (ignorez, flat, wireframe)
        // Set color
        // Call original with override
    }
    */
}

// Surface::LockCursor
// VA: 0x34E26620
// Signature: void __thiscall hkLockCursor(void* ecx)
namespace LockCursor {
    constexpr uintptr_t ADDRESS = 0x34E26620;
    constexpr uintptr_t ORIGINAL_PTR = 0x43AFE634;
    constexpr uintptr_t MENU_OPEN = 0x43AFD094;
    constexpr uintptr_t UNLOCK_FLAG = 0x43AFE638;
    constexpr int UNLOCK_CURSOR_INDEX = 0x42; // 66 decimal, 0x108 bytes

    /*
    void __thiscall hkLockCursor(void* ecx) {
        if (*(bool*)0x43AFD094) { // menu open?
            *(bool*)0x43AFE638 = true;
            surface->UnlockCursor(); // vtable[66]
            return;
        }
        *(bool*)0x43AFE638 = false;
        original(ecx);
    }
    */
}

// ===================== Netvar Proxies =====================

namespace Proxies {
    // CCSPlayer::m_angEyeAngles[0] - Pitch
    // VA: 0x34E24BF0
    constexpr uintptr_t PITCH = 0x34E24BF0;
    /*
    void __cdecl PitchProxy(RecvProxyData& data, void* entity, void* out) {
        int entIndex = GetEntityIndex(entity); // via engine?
        LagRecord* rec = GetLagRecord(entIndex);
        rec->eye_angles.pitch = data.value._float;
        // Clamp?
        *(float*)out = data.value._float;
    }
    */

    // CCSPlayer::m_angEyeAngles[1] - Yaw
    // VA: 0x34E24D20
    constexpr uintptr_t YAW = 0x34E24D20;

    // CCSPlayer::m_flLowerBodyYawTarget
    // VA: 0x34E24EC0
    constexpr uintptr_t LBY = 0x34E24EC0;
    /*
    void __cdecl LBYProxy(RecvProxyData& data, void* entity, void* out) {
        LagRecord* rec = GetLagRecord(entIndex);
        rec->lower_body_yaw = data.value._float;
        rec->lby_update_time = curtime; // ?
        *(float*)out = data.value._float;
    }
    */

    // CBasePlayer::m_fFlags
    // VA: 0x34E24F40
    constexpr uintptr_t FLAGS = 0x34E24F40;

    // CCSPlayer::m_flFlashDuration
    // VA: 0x34E24B90
    constexpr uintptr_t FLASH = 0x34E24B90;

    // CCSPlayer::m_flThirdpersonRecoil
    // VA: 0x34E24FC0
    constexpr uintptr_t RECOIL = 0x34E24FC0;

    // CSmokeGrenadeProjectile::m_nSmokeEffectTickBegin
    // VA: 0x34E24BC0
    constexpr uintptr_t SMOKE_TICK = 0x34E24BC0;

    // CViewRender::OnRenderStart
    // VA: 0x34E25040
    constexpr uintptr_t ON_RENDER_START = 0x34E25040;
    constexpr uintptr_t ON_RENDER_START_PTR = 0x43AFE0E8;
}

// ===================== Aimbot Functions =====================

namespace Aimbot {
    // Hitchance & Spread seed calc
    // VA: 0x34E282D0
    constexpr uintptr_t HITCHANCE = 0x34E282D0;
    /*
    bool CalculateHitchance(CUserCmd* cmd, QAngle angles, float required) {
        int hits = 0;
        for (int i = 0; i < 256; i++) {
            RandomSeed(i + 1);
            float a = RandomFloat(0, 2*M_PI);
            float b = RandomFloat(0, 2*M_PI);
            // ... spread calc
            // Trace
            if (hit) hits++;
        }
        return (hits / 256.0f * 100.0f) >= required;
    }
    */

    // Best target & FOV scanner
    // VA: 0x34E28D40
    constexpr uintptr_t GET_BEST_TARGET = 0x34E28D40;

    // AutoWall penetration simulator
    // VA: 0x34E29040
    constexpr uintptr_t AUTOWALL = 0x34E29040;

    // Multipoint hitbox scanner
    // VA: 0x34E29B90
    constexpr uintptr_t MULTIPOINT = 0x34E29B90;
}

// ===================== AntiAim =====================

namespace AntiAim {
    // Pitch anti-aim
    // VA: 0x34E2A910
    constexpr uintptr_t PITCH = 0x34E2A910;
    /*
    float CalculatePitch(int mode) {
        switch(mode) {
            case 1: return 89.0f; // emotion
            case 2: return -89.0f; // up
            case 3: return 0.0f; // zero
            case 4: return 180.0f; // fake
        }
    }
    */

    // Yaw anti-aim
    // VA: 0x34E2AF90
    constexpr uintptr_t YAW = 0x34E2AF90;

    // Fakelag choke
    // VA: 0x34E2B720
    constexpr uintptr_t FAKELAG = 0x34E2B720;
    /*
    void ProcessFakeLag(CUserCmd* cmd, bool& send_packet) {
        static int choked = 0;
        choked++;
        if (choked >= 6) { choked = 0; send_packet = true; }
        else send_packet = false;
    }
    */
}

// ===================== Skinchanger =====================

namespace Skinchanger {
    // FrameStageNotify skin update
    // VA: 0x34E2C580
    constexpr uintptr_t UPDATE = 0x34E2C580;

    // GloveChanger
    // VA: 0x34E1DFC9
    constexpr uintptr_t GLOVE = 0x34E1DFC9;
    /*
    Disasm:
    34e1dfc9: je 0x34e1dff6
    34e1dfcb: mov cl, [0x76ed9ce0]           ; Encrypted glove model path
    34e1dfd1: mov eax, [0x43aff218]          ; profile_t XOR key
    34e1dfd6: xor cl, [eax]                  ; Decrypt byte
    ...
    34e1e003: call strstr
    34e1e026: call strchr
    34e1e046: call eax                       ; MDLCache::FindMDL
    */

    // PaintkitChanger
    // VA: 0x34E1E2DA
    constexpr uintptr_t PAINTKIT = 0x34E1E2DA;

    // FillSkins
    // VA: 0x34E1F14E
    constexpr uintptr_t FILL_SKINS = 0x34E1F14E;
}

// ===================== Visuals =====================

namespace Visuals {
    // Main ESP loop
    // VA: 0x34E1D1D0
    constexpr uintptr_t ESP_LOOP = 0x34E1D1D0;

    // CreateFonts
    // VA: 0x34E30FE4
    constexpr uintptr_t CREATE_FONTS = 0x34E30FE4;

    // GetRadarHudElement
    // VA: 0x34E31D7B
    constexpr uintptr_t RADAR = 0x34E31D7B;
}

// ===================== Config & UI =====================

namespace ConfigUI {
    constexpr uintptr_t GEN_CONFIG_PATH = 0x34E34E90;
    constexpr uintptr_t DUMP_CONFIG = 0x34E357C5;
    constexpr uintptr_t GET_CONFIG_LIST = 0x34E35983;
    constexpr uintptr_t CONFIG_ACTION = 0x34E34746;

    // UI elements
    constexpr uintptr_t DRAW_MENU_HEADER = 0x34E37D73;
    constexpr uintptr_t DRAW_CHECKBOX = 0x34E372C5;
    constexpr uintptr_t DRAW_SLIDER = 0x34E37074;
    constexpr uintptr_t DRAW_KEYBIND = 0x34E38185;
    constexpr uintptr_t DRAW_COMBOBOX = 0x34E388A4;
    constexpr uintptr_t DRAW_TAB_BAR = 0x34E38FB2;
    constexpr uintptr_t DRAW_BUTTON = 0x34E39396;

    // Slider formatters
    constexpr uintptr_t SLIDER_FMT = 0x34E36CC5;
    constexpr uintptr_t SLIDER_FMT2 = 0x34E36C45;
    constexpr uintptr_t SLIDER_FMT3 = 0x34E36BC5;
    constexpr uintptr_t SLIDER_FMT4 = 0x34E36B45;
}

// ===================== Prediction =====================

namespace Prediction {
    constexpr uintptr_t RUN_COMMAND = 0x34E314B0;
    constexpr uintptr_t SETUP_MOVE = 0x34E26880;
    constexpr uintptr_t IN_PREDICTION = 0x34E26500;
    constexpr uintptr_t WRITE_USERCMD_DELTA = 0x34E268C0;
}

// ===================== Init =====================

namespace Init {
    constexpr uintptr_t NETVAR_INIT = 0x34E1D890;
    constexpr uintptr_t CONFIG_LIST_REFRESH = 0x34E2B410;
    constexpr uintptr_t PANIC = 0x34E25530;
    constexpr uintptr_t WND_PROC = 0x34E33D60;
}

} // namespace Aimware::Reversed::Functions
