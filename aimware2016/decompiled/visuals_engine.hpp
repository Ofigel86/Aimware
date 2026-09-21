#pragma once
#include "decompiled_sdk.hpp"
#include <vector>
#include <string>

namespace Aimware2016Decompiled {

enum class ESPBoxType {
    NONE = 0,
    BOX_2D,
    BOX_3D,
    BOX_CORNERS
};

enum class ChamsMaterial {
    NONE = 0,
    FLAT,
    TEXTURED,
    METALLIC,
    GLOW
};

struct ESPConfig {
    bool enabled = true;
    bool box = true;
    ESPBoxType boxType = ESPBoxType::BOX_2D;
    bool healthBar = true;
    bool name = true;
    bool weapon = true;
    bool skeleton = false;
    bool snapLines = false;
    bool glow = false;
    bool chams = true;
    bool chamsIgnoreZ = true;
    ChamsMaterial chamsMaterial = ChamsMaterial::FLAT;
    bool noSmoke = true;
    bool noFlash = true;
    bool noRecoil = false;
    bool thirdPerson = false;
};

struct PlayerESPInfo {
    int index = 0;
    Vector origin;
    Vector mins, maxs;
    int health = 0;
    int team = 0;
    std::string name;
    std::string weapon;
    bool isVisible = false;
    bool isDormant = false;
    float distance = 0.f;
    Matrix3x4 boneMatrix[128];
};

class VisualsEngine {
public:
    static VisualsEngine& Instance() {
        static VisualsEngine inst;
        return inst;
    }

    // Hook handlers
    static void __fastcall hkEngineVGUIPaint(void* ecx, void* edx, int mode);
    static void __fastcall hkDrawModel(void* ecx, void* edx, void* results, void* info, Matrix3x4* bone_to_world, float* weights, float* origin, int flags);
    static void __fastcall hkLockCursor(void* ecx, void* edx);

    // ESP
    static void DrawESP();
    static void DrawPlayerESP(const PlayerESPInfo& info);
    static void DrawWorldESP();

    // Chams
    static void ApplyChams(void* entity, Matrix3x4* boneMatrix);
    static void OverrideMaterial(ChamsMaterial mat, bool ignoreZ, const float* color);

    // Effects
    static void NoSmoke();
    static void NoFlash();
    static void ThirdPerson();

    // Menu
    static bool IsMenuOpen();
    static void SetMenuOpen(bool open);

    ESPConfig config;

private:
    VisualsEngine() = default;
    static bool GetPlayerBox(const PlayerESPInfo& info, int& x, int& y, int& w, int& h);
    static bool WorldToScreen(const Vector& world, Vector& screen);
};

} // namespace Aimware2016Decompiled
