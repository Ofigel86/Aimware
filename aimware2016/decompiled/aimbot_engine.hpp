#pragma once
#include "decompiled_sdk.hpp"
#include <vector>
#include <optional>
#include <algorithm>

namespace Aimware2016Decompiled {

enum class Hitbox {
    HEAD = 0,
    NECK,
    PELVIS,
    STOMACH,
    THORAX,
    LOWER_CHEST,
    UPPER_CHEST,
    RIGHT_THIGH,
    LEFT_THIGH,
    RIGHT_CALF,
    LEFT_CALF,
    RIGHT_FOOT,
    LEFT_FOOT,
    RIGHT_HAND,
    LEFT_HAND,
    RIGHT_UPPER_ARM,
    RIGHT_FOREARM,
    LEFT_UPPER_ARM,
    LEFT_FOREARM,
    MAX
};

struct AimbotConfig {
    bool enabled = true;
    float fov = 30.f;
    float hitchance = 50.f;
    float minDamage = 30.f;
    bool autoShoot = true;
    bool silent = true;
    bool multipoint = true;
    float multipointScale = 0.8f;
    bool autowall = true;
    bool smokeCheck = false;
    bool flashCheck = true;
};

struct TargetInfo {
    int entityIndex = -1;
    Vector hitboxPos;
    Hitbox hitbox;
    float fov = 180.f;
    float damage = 0.f;
    float distance = 0.f;
    bool visible = false;
};

class AimbotEngine {
public:
    static AimbotEngine& Instance() {
        static AimbotEngine inst;
        return inst;
    }

    // Main hook handler - ClientMode::CreateMove
    static bool __stdcall hkCreateMove(float input_sample_frametime, CUserCmd* cmd);

    // Hitchance with proper seed calculation
    static bool CalculateHitchance(CUserCmd* cmd, const QAngle& angles, float required_hitchance, int weaponId = 0);

    // Target selection
    static int GetBestTarget(CUserCmd* cmd, Vector& out_target_pos);
    static std::optional<TargetInfo> SelectBestTarget(CUserCmd* cmd);

    // Autowall
    static float SimulateAutoWall(void* local_player, const Vector& target_pos, int hitbox = 0);
    static bool IsVisible(const Vector& start, const Vector& end, void* skip1 = nullptr, void* skip2 = nullptr);

    // Hitbox scanning
    static void ScanHitboxPoints(void* entity, int hitbox_index, std::vector<Vector>& out_points, float scale = 0.8f);
    static Vector GetHitboxPosition(void* entity, int hitbox);
    static bool GetHitboxPoints(void* entity, int hitbox, std::vector<Vector>& points);

    // Utility
    static float GetSpread(int random_seed);
    static void FixMovement(CUserCmd* cmd, const QAngle& oldAngles);

    AimbotConfig config;

private:
    AimbotEngine() = default;
};

} // namespace Aimware2016Decompiled
