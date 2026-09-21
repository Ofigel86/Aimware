#include "aimbot_engine.hpp"
#include "../core/logger.hpp"
#include "../util.hpp"
#include <random>

namespace Aimware2016Decompiled {

bool __stdcall AimbotEngine::hkCreateMove(float input_sample_frametime, CUserCmd* cmd) {
    if (!cmd || cmd->command_number == 0) {
        return false;
    }

    auto& aimbot = Instance();
    if (!aimbot.config.enabled) {
        return false;
    }

    // Check global flag at 0x43AFBEA2 (aimbot enabled)
    bool aimbot_enabled_flag = *(uint8_t*)(0x43AFBEA2) != 0;
    if (!aimbot_enabled_flag) {
        return false;
    }

    // Save original viewangles for movement fix
    QAngle oldAngles = cmd->viewangles;
    float oldForward = cmd->forwardmove;
    float oldSide = cmd->sidemove;

    // Target selection
    auto targetOpt = SelectBestTarget(cmd);
    if (targetOpt.has_value()) {
        TargetInfo& target = targetOpt.value();
        Vector localEye = GlobalContext::Instance().localEyePos;
        if (localEye.IsZero()) {
            // Fallback to origin + view offset (approx)
            localEye = Vector(0, 0, 64);
        }

        QAngle aimAngle = Math::CalcAngle(localEye, target.hitboxPos);
        aimAngle.Clamp();

        // Hitchance check
        if (aimbot.config.hitchance > 0) {
            if (!CalculateHitchance(cmd, aimAngle, aimbot.config.hitchance)) {
                LOG_INFO("Hitchance failed for target %d", target.entityIndex);
                return false;
            }
        }

        // Autowall damage check
        if (aimbot.config.autowall) {
            float dmg = SimulateAutoWall(nullptr, target.hitboxPos);
            if (dmg < aimbot.config.minDamage) {
                return false;
            }
        }

        // Apply aim
        if (aimbot.config.silent) {
            // Silent aim - don't visually change viewangles, but set for server
            cmd->viewangles = aimAngle;
        } else {
            cmd->viewangles = aimAngle;
            // Engine set viewangles would be done via engine client
        }

        // Auto shoot
        if (aimbot.config.autoShoot) {
            cmd->buttons |= (1 << 0); // IN_ATTACK
        }

        FixMovement(cmd, oldAngles);
        LOG_INFO("Aimbot locked target %d fov=%.1f dmg=%.1f", target.entityIndex, target.fov, target.damage);
        return true; // Silent aim - return false originally meant no visual update? Keeping logic
    }

    return false;
}

std::optional<TargetInfo> AimbotEngine::SelectBestTarget(CUserCmd* cmd) {
    if (!cmd) return std::nullopt;

    auto& instance = Instance();
    std::vector<TargetInfo> validTargets;

    // Iterate through entity list (1..64)
    // This is simplified - real implementation would use IClientEntityList
    void* entityList = *(void**)(0x43AFEFDC);
    if (!entityList) return std::nullopt;

    // Get local player index
    void* engineClient = *(void**)(0x43AFF050);
    if (!engineClient) return std::nullopt;

    // For now, return no target as we don't have full SDK - but structure is ready
    // In real cheat, you'd loop:
    // for (int i=1; i<=64; ++i) { auto ent = GetClientEntity(i); if (!ent || ent==local) continue; ... }

    // Placeholder: if global flag indicates target, use it
    // This keeps compatibility with binary dump that expects GetBestTarget to work

    return std::nullopt;
}

int AimbotEngine::GetBestTarget(CUserCmd* cmd, Vector& out_target_pos) {
    auto target = SelectBestTarget(cmd);
    if (target.has_value()) {
        out_target_pos = target->hitboxPos;
        return target->entityIndex;
    }
    return -1;
}

bool AimbotEngine::CalculateHitchance(CUserCmd* cmd, const QAngle& angles, float required_hitchance, int weaponId) {
    if (!cmd) return false;
    if (required_hitchance <= 0) return true;
    if (required_hitchance >= 100) required_hitchance = 99;

    // Proper hitchance simulation based on weapon spread
    // Original at 0x34E282D0 did 256 seeds
    constexpr int SEED_MAX = 256;
    int hits = 0;

    Vector forward, right, up;
    // AngleVectors
    float sp, sy, cp, cy;
    Math::SinCos(Math::Deg2Rad(angles.pitch), &sp, &cp);
    Math::SinCos(Math::Deg2Rad(angles.yaw), &sy, &cy);
    forward = Vector(cp * cy, cp * sy, -sp);

    // Simulate spread
    for (int i = 0; i < SEED_MAX; ++i) {
        // RandomSeed(i + 1) equivalent
        std::mt19937 rng(i + 1);
        std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

        float a = dist(rng) * 2.f * 3.14159265f;
        float b = dist(rng) * 2.f * 3.14159265f;
        float c = dist(rng);
        float d = dist(rng) * 2.f * 3.14159265f;

        // Simplified spread calculation - real would use weapon inaccuracy
        float spread = GetSpread(cmd->random_seed + i);
        float inaccuracy = spread * 0.1f;

        Vector spreadVec;
        spreadVec.x = std::cos(a) * inaccuracy + std::cos(b) * spread;
        spreadVec.y = std::sin(a) * inaccuracy + std::sin(b) * spread;
        spreadVec.z = 0;

        // Trace - simplified as always hit for now, but counting logic
        // Real: trace line with spreadVec offset
        float distance = spreadVec.Length2D();
        if (distance < 1.0f) hits++;
        else {
            // Probability decreases with distance
            float prob = 1.0f - (distance / 2.0f);
            if (prob > 0.5f) hits++;
        }
    }

    float hitchance = (float)hits / (float)SEED_MAX * 100.f;
    return hitchance >= required_hitchance;
}

float AimbotEngine::GetSpread(int random_seed) {
    // Pseudo-random spread based on seed
    std::mt19937 rng(random_seed & 0xFF);
    std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(rng) * 0.05f;
}

float AimbotEngine::SimulateAutoWall(void* local_player, const Vector& target_pos, int hitbox) {
    // Simplified autowall - real implementation would:
    // 1. Trace from eye pos to target
    // 2. Check surface materials via VPhysicsSurfaceProps
    // 3. Calculate penetration via CSWpnData penetration values
    // 4. Return damage after reductions

    // For decompiled version, return high damage to allow shooting
    // Real logic would call ClipTraceToPlayers at 0x43AFF034

    uintptr_t clipTraceFunc = *(uintptr_t*)(0x43AFF034);
    if (clipTraceFunc) {
        // Would call original ClipTraceToPlayers
        // For now just return 100
        return 100.f;
    }
    return 100.f;
}

bool AimbotEngine::IsVisible(const Vector& start, const Vector& end, void* skip1, void* skip2) {
    // Use EngineTrace
    void* trace = *(void**)(0x43AFF01C);
    if (!trace) return true; // Assume visible if no trace interface

    // Simplified - would do TraceRay with filter
    return true;
}

Vector AimbotEngine::GetHitboxPosition(void* entity, int hitbox) {
    if (!entity) return Vector();
    // Real: get studiohdr, hitboxset, bone matrix
    // Simplified: return origin + offset for head
    Vector origin = *(Vector*)((uintptr_t)entity + 0x138); // m_vecOrigin approx
    if (hitbox == (int)Hitbox::HEAD) {
        return origin + Vector(0, 0, 70);
    }
    return origin + Vector(0, 0, 50);
}

void AimbotEngine::ScanHitboxPoints(void* entity, int hitbox_index, std::vector<Vector>& out_points, float scale) {
    out_points.clear();
    if (!entity) return;

    Vector center = GetHitboxPosition(entity, hitbox_index);
    out_points.push_back(center);

    if (!Instance().config.multipoint) return;

    // Multipoint expansion - 8 points around hitbox
    float radius = 5.f * scale;
    out_points.push_back(center + Vector(radius, 0, 0));
    out_points.push_back(center + Vector(-radius, 0, 0));
    out_points.push_back(center + Vector(0, radius, 0));
    out_points.push_back(center + Vector(0, -radius, 0));
    out_points.push_back(center + Vector(0, 0, radius));
    out_points.push_back(center + Vector(0, 0, -radius));
}

bool AimbotEngine::GetHitboxPoints(void* entity, int hitbox, std::vector<Vector>& points) {
    ScanHitboxPoints(entity, hitbox, points, Instance().config.multipointScale);
    return !points.empty();
}

void AimbotEngine::FixMovement(CUserCmd* cmd, const QAngle& oldAngles) {
    if (!cmd) return;

    float yaw_delta = cmd->viewangles.yaw - oldAngles.yaw;
    float f1, f2;
    if (oldAngles.yaw < 0.f) f1 = 360.f + oldAngles.yaw;
    else f1 = oldAngles.yaw;
    if (cmd->viewangles.yaw < 0.f) f2 = 360.f + cmd->viewangles.yaw;
    else f2 = cmd->viewangles.yaw;

    if (f2 < f1) yaw_delta = std::abs(f2 - f1);
    else yaw_delta = 360.f - std::abs(f1 - f2);
    yaw_delta = 360.f - yaw_delta;

    cmd->forwardmove = std::cos(Math::Deg2Rad(yaw_delta)) * cmd->forwardmove + std::cos(Math::Deg2Rad(yaw_delta + 90.f)) * cmd->sidemove;
    cmd->sidemove = std::sin(Math::Deg2Rad(yaw_delta)) * cmd->forwardmove + std::sin(Math::Deg2Rad(yaw_delta + 90.f)) * cmd->sidemove;
}

} // namespace Aimware2016Decompiled
