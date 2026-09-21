#include "antiaim_engine.hpp"
#include "../core/logger.hpp"

namespace Aimware2016Decompiled {

float AntiAimEngine::lastLBYUpdate = 0.f;
float AntiAimEngine::nextLBYUpdate = 0.f;
int AntiAimEngine::chokedTicks = 0;
float AntiAimEngine::spinYaw = 0.f;

float AntiAimEngine::CalculatePitch(int mode) {
    switch ((PitchMode)mode) {
        case PitchMode::EMOTION: return 89.f;
        case PitchMode::UP: return -89.f;
        case PitchMode::ZERO: return 0.f;
        case PitchMode::FAKE: return 180.f;
        case PitchMode::CUSTOM: return Instance().config.pitchMode == PitchMode::CUSTOM ? 89.f : 0.f;
        default: return 0.f;
    }
}

float AntiAimEngine::CalculateYaw(int mode, CUserCmd* cmd, bool is_fake) {
    auto& cfg = Instance().config;

    if (is_fake) {
        // Fake yaw logic
        switch (cfg.fakeYawMode) {
            case YawMode::BACKWARD: return 180.f;
            case YawMode::SIDEWAYS: return 90.f;
            case YawMode::STATIC: return -90.f;
            default: return 90.f;
        }
    }

    // Real yaw logic
    switch ((YawMode)mode) {
        case YawMode::BACKWARD:
            return 180.f;

        case YawMode::SIDEWAYS:
            return 90.f;

        case YawMode::SPINBOT: {
            spinYaw += cfg.spinSpeed;
            if (spinYaw > 180.f) spinYaw -= 360.f;
            if (spinYaw < -180.f) spinYaw += 360.f;
            return spinYaw;
        }

        case YawMode::JITTER: {
            static bool flip = false;
            flip = !flip;
            return flip ? cfg.jitterRange : -cfg.jitterRange;
        }

        case YawMode::STATIC:
            return 0.f;

        case YawMode::LBY_BREAK: {
            if (IsBreakingLBY(cmd)) {
                return GetLBYBreakDelta();
            }
            return 180.f;
        }

        default:
            return 180.f;
    }
}

void AntiAimEngine::ProcessFakeLag(CUserCmd* cmd, bool& send_packet) {
    if (!cmd) return;

    auto& cfg = Instance().config;
    if (!cfg.fakeLagEnabled || !cfg.enabled) {
        send_packet = true;
        return;
    }

    // Original at 0x34E2B720
    chokedTicks++;

    int maxChoke = cfg.fakeLag;
    if (maxChoke < 1) maxChoke = 1;
    if (maxChoke > 14) maxChoke = 14; // Max choke limit

    // Break LBY logic - always send when LBY updates
    if (IsBreakingLBY(cmd)) {
        send_packet = true;
        chokedTicks = 0;
        UpdateLBYTimer();
        LOG_INFO("Fakelag: LBY break - sending packet");
        return;
    }

    if (chokedTicks >= maxChoke) {
        chokedTicks = 0;
        send_packet = true;
    } else {
        send_packet = false;
    }
}

QAngle AntiAimEngine::CalculateAntiAimAngles(CUserCmd* cmd, bool& send_packet) {
    QAngle angles = cmd->viewangles;
    auto& cfg = Instance().config;

    if (!cfg.enabled) {
        send_packet = true;
        return angles;
    }

    // Pitch
    angles.pitch = CalculatePitch((int)cfg.pitchMode);

    // Yaw - handle real vs fake
    bool isFake = !send_packet; // If we're choking, next is fake?
    // Actually, logic: when send_packet is false, we are building fake angle for next
    // But for simplicity, calculate both and apply based on choke

    float realYaw = CalculateYaw((int)cfg.yawMode, cmd, false);
    float fakeYaw = CalculateYaw((int)cfg.fakeYawMode, cmd, true);

    // At targets - find closest enemy and aim away
    if (cfg.atTargets) {
        // Simplified: would find closest enemy and set yaw opposite
        // For now just use calculated yaw
    }

    // Edge anti-aim - would trace walls
    if (cfg.edge) {
        // Trace around to find wall edge
    }

    if (send_packet) {
        angles.yaw += realYaw;
    } else {
        angles.yaw += fakeYaw;
    }

    angles.Clamp();
    return angles;
}

float AntiAimEngine::GetLBYBreakDelta() {
    // Return delta that breaks LBY (usually 90-120 deg from last)
    return 120.f;
}

bool AntiAimEngine::IsBreakingLBY(CUserCmd* cmd) {
    // LBY updates every 1.1 seconds when not moving, or 0.22 when moving
    // Check velocity and time
    static float lastCheck = 0.f;
    // Simplified check - every 1.1 sec
    // Real would check local player velocity and server time
    return false; // Placeholder
}

void AntiAimEngine::UpdateLBYTimer() {
    lastLBYUpdate = 0.f; // Would use globalvars curtime
    nextLBYUpdate = lastLBYUpdate + 1.1f;
}

} // namespace Aimware2016Decompiled
