#include "visuals_engine.hpp"
#include "../core/logger.hpp"

namespace Aimware2016Decompiled {

void __fastcall VisualsEngine::hkEngineVGUIPaint(void* ecx, void* edx, int mode) {
    // Call original first
    uintptr_t original = *(uintptr_t*)(0x43AFE63C);
    if (original) {
        // Original is at 0x34E26660
        // Would call: ((void(__thiscall*)(void*, int))original)(ecx, mode);
    }

    // Only draw when UI panels are being rendered (PAINT_UIPANELS = 1)
    if (mode & 1) {
        DrawESP();
    }
}

void __fastcall VisualsEngine::hkDrawModel(void* ecx, void* edx, void* results, void* info, Matrix3x4* bone_to_world, float* weights, float* origin, int flags) {
    // Check chams flag at 0x43AFCCB2
    bool chamsEnabled = *(uint8_t*)(0x43AFCCB2) != 0;
    auto& visuals = Instance();

    if (chamsEnabled && visuals.config.chams && bone_to_world) {
        // Get entity from info - simplified
        // Real: info->renderable, check team, etc
        // Apply chams material override
        // Would get material from 0x76ED9EA0 (decrypted string)

        // For decompiled version, log
        // LOG_INFO("Chams: DrawModel called");

        // Override material
        // OverrideMaterial(visuals.config.chamsMaterial, visuals.config.chamsIgnoreZ, nullptr);
    }

    // Call original DrawModel at 0x43AFE17C
    uintptr_t original = *(uintptr_t*)(0x43AFE17C);
    if (original) {
        // ((void(__thiscall*)(void*, void*, void*, Matrix3x4*, float*, float*, int))original)(ecx, results, info, bone_to_world, weights, origin, flags);
    }
}

void __fastcall VisualsEngine::hkLockCursor(void* ecx, void* edx) {
    // Check menu_open flag at 0x43AFD094
    bool menuOpen = *(uint8_t*)(0x43AFD094) != 0;
    auto& visuals = Instance();

    if (menuOpen || visuals.IsMenuOpen()) {
        // Unlock cursor - call ISurface::UnlockCursor (vtable index 0x42 / 66)
        void* surface = *(void**)(0x43AFF098);
        if (surface) {
            // ((void(__thiscall*)(void*))(*(uintptr_t**)surface)[66])(surface);
            // For now, set flag at 0x43AFE638 to indicate unlocked
            *(int*)(0x43AFE638) = 1;
        }
        return;
    }

    // Call original LockCursor
    uintptr_t original = *(uintptr_t*)(0x43AFE634);
    if (original) {
        // ((void(__thiscall*)(void*))original)(ecx);
        *(int*)(0x43AFE638) = 0;
    }
}

void VisualsEngine::DrawESP() {
    auto& visuals = Instance();
    if (!visuals.config.enabled) return;

    auto& ctx = GlobalContext::Instance();
    if (!ctx.render) {
        ctx.UpdateFromFixed();
    }

    // NoSmoke effect
    if (visuals.config.noSmoke) {
        NoSmoke();
    }

    // Get entity list and loop
    void* entityList = *(void**)(0x43AFEFDC);
    if (!entityList) return;

    // Simplified ESP loop - would iterate 1..64 entities
    // For each player, get info and draw

    // LOG_INFO("ESP: Drawing frame");

    // Example: draw watermark
    // Would use ISurface drawing functions
}

void VisualsEngine::DrawPlayerESP(const PlayerESPInfo& info) {
    if (info.isDormant) return;
    auto& cfg = Instance().config;

    int x, y, w, h;
    if (!GetPlayerBox(info, x, y, w, h)) return;

    // Box
    if (cfg.box) {
        switch (cfg.boxType) {
            case ESPBoxType::BOX_2D:
                // Draw 2D box
                break;
            case ESPBoxType::BOX_3D:
                // Draw 3D box using mins/maxs
                break;
            case ESPBoxType::BOX_CORNERS:
                // Draw corner box
                break;
            default: break;
        }
    }

    // Health bar
    if (cfg.healthBar) {
        int health = std::clamp(info.health, 0, 100);
        int barHeight = h * health / 100;
        // Draw health bar
    }

    // Name
    if (cfg.name && !info.name.empty()) {
        // Draw name above box
    }

    // Weapon
    if (cfg.weapon && !info.weapon.empty()) {
        // Draw weapon below box
    }

    // Skeleton
    if (cfg.skeleton) {
        // Draw skeleton using boneMatrix
        for (int i = 0; i < 128; ++i) {
            // Draw bones
        }
    }
}

void VisualsEngine::DrawWorldESP() {
    // Draw dropped weapons, grenades, etc
}

void VisualsEngine::ApplyChams(void* entity, Matrix3x4* boneMatrix) {
    if (!entity || !boneMatrix) return;
    auto& cfg = Instance().config;

    // Get material override string from 0x76ED9EA0 (XOR encrypted)
    // Decrypt using profile key at 0x43AFF218
    uintptr_t profile = *(uintptr_t*)(0x43AFF218);
    if (!profile) return;

    int xorKey = *(int*)profile;
    // Decrypt material string...

    // Apply material
    OverrideMaterial(cfg.chamsMaterial, cfg.chamsIgnoreZ, nullptr);
}

void VisualsEngine::OverrideMaterial(ChamsMaterial mat, bool ignoreZ, const float* color) {
    // Would get IMaterialSystem at 0x43AFF000
    void* matSystem = *(void**)(0x43AFF000);
    if (!matSystem) return;

    // Create or find material
    // Set material vars: ignorez, flat, wireframe, etc
    // Apply color modulation
}

void VisualsEngine::NoSmoke() {
    // Zero out smoke count at 0x43AFF0B4
    uintptr_t smokeCountAddr = *(uintptr_t*)(0x43AFF0B4);
    if (smokeCountAddr) {
        *(int*)smokeCountAddr = 0;
    }

    // Also remove smoke materials - would set NoDraw
}

void VisualsEngine::NoFlash() {
    // Hook m_flFlashDuration proxy already does this
    // Set flash alpha to 0
}

void VisualsEngine::ThirdPerson() {
    // Set m_bCameraInThirdPerson at 0x43AFEF54
    // Set m_vecViewOffset at 0x43AFEF4C
}

bool VisualsEngine::IsMenuOpen() {
    return *(uint8_t*)(0x43AFD094) != 0;
}

void VisualsEngine::SetMenuOpen(bool open) {
    *(uint8_t*)(0x43AFD094) = open ? 1 : 0;
}

bool VisualsEngine::GetPlayerBox(const PlayerESPInfo& info, int& x, int& y, int& w, int& h) {
    // World to screen for mins/maxs
    // Simplified
    Vector screenOrigin;
    if (!WorldToScreen(info.origin, screenOrigin)) return false;

    // Estimate box size based on distance
    float dist = info.distance;
    if (dist <= 0) dist = 100;
    h = (int)(4000 / dist);
    w = h / 2;
    x = (int)screenOrigin.x - w / 2;
    y = (int)screenOrigin.y - h;

    return true;
}

bool VisualsEngine::WorldToScreen(const Vector& world, Vector& screen) {
    // Use VDebugOverlay at 0x43AFF088 or engine's WorldToScreen
    // Simplified
    screen = world; // Placeholder
    return true;
}

} // namespace Aimware2016Decompiled
