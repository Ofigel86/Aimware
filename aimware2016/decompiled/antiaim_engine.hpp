#pragma once
#include "decompiled_sdk.hpp"

namespace Aimware2016Decompiled {

enum class PitchMode {
    NONE = 0,
    EMOTION = 1,    // 89 deg
    UP = 2,         // -89 deg
    ZERO = 3,       // 0 deg
    FAKE = 4,       // 180 deg
    CUSTOM = 5
};

enum class YawMode {
    NONE = 0,
    BACKWARD = 1,   // 180
    SIDEWAYS = 2,   // 90 / -90
    SPINBOT = 3,    // spinning
    JITTER = 4,     // jitter
    STATIC = 5,
    LBY_BREAK = 6
};

struct AntiAimConfig {
    bool enabled = false;
    PitchMode pitchMode = PitchMode::EMOTION;
    YawMode yawMode = YawMode::BACKWARD;
    YawMode fakeYawMode = YawMode::SIDEWAYS;
    bool atTargets = true;
    bool edge = false;
    bool knifeHeld = false;
    int fakeLag = 6;
    bool fakeLagEnabled = false;
    float spinSpeed = 30.f;
    float jitterRange = 45.f;
};

class AntiAimEngine {
public:
    static AntiAimEngine& Instance() {
        static AntiAimEngine inst;
        return inst;
    }

    static float CalculatePitch(int mode);
    static float CalculateYaw(int mode, CUserCmd* cmd, bool is_fake);
    static void ProcessFakeLag(CUserCmd* cmd, bool& send_packet);

    // Enhanced methods
    static QAngle CalculateAntiAimAngles(CUserCmd* cmd, bool& send_packet);
    static float GetLBYBreakDelta();
    static bool IsBreakingLBY(CUserCmd* cmd);
    static void UpdateLBYTimer();

    AntiAimConfig config;

private:
    AntiAimEngine() = default;
    static float lastLBYUpdate;
    static float nextLBYUpdate;
    static int chokedTicks;
    static float spinYaw;
};

} // namespace Aimware2016Decompiled
