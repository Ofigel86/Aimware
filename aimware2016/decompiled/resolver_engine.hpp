#pragma once
#include "decompiled_sdk.hpp"
#include <unordered_map>
#include <deque>

namespace Aimware2016Decompiled {

enum class ResolverMode {
    OFF = 0,
    LBY = 1,
    BRUTEFORCE = 2,
    ANIM = 3,
    ADVANCED = 4
};

struct PlayerResolveInfo {
    int index = 0;
    float lastLBY = 0.f;
    float lastMovingLBY = 0.f;
    float lastUpdateTime = 0.f;
    int missedShots = 0;
    ResolverMode mode = ResolverMode::LBY;
    float resolvedYaw = 0.f;
    std::deque<float> lbyDeltas;
    bool isMoving = false;
    bool isBreakingLBY = false;
    Vector lastVelocity;
    float simulationTime = 0.f;
};

class ResolverEngine {
public:
    static ResolverEngine& Instance() {
        static ResolverEngine inst;
        return inst;
    }

    // Original proxy hooks
    static void __cdecl OnPitchProxy(float* pitch, int ent_index);
    static void __cdecl OnYawProxy(float* yaw, int ent_index);
    static void __cdecl OnLBYProxy(float* lby, int ent_index);
    static void __fastcall hkOnRenderStart(void* ecx, void* edx);

    // Enhanced resolver logic
    static float ResolveYaw(int ent_index, float original_yaw, float lby);
    static void UpdatePlayerInfo(int ent_index, float lby, float yaw, const Vector& velocity);
    static void OnMissedShot(int ent_index);
    static void ResetPlayer(int ent_index);

    static PlayerResolveInfo* GetPlayerInfo(int ent_index);

    bool enabled = true;
    ResolverMode mode = ResolverMode::ADVANCED;

private:
    ResolverEngine() {
        for (int i = 0; i < 65; ++i) {
            players_[i].index = i;
        }
    }

    PlayerResolveInfo players_[65];
    std::unordered_map<int, int> bruteforceStage_;
};

} // namespace Aimware2016Decompiled
