#include "resolver_engine.hpp"
#include "../core/logger.hpp"

namespace Aimware2016Decompiled {

void __cdecl ResolverEngine::OnPitchProxy(float* pitch, int ent_index) {
    if (!pitch) return;

    // Store pitch in lag records - original behavior at 0x34E24BF0
    uintptr_t records_base = 0;
    try {
        records_base = *(uintptr_t*)(0x43AF7A84);
    } catch (...) { return; }

    if (records_base && ent_index >= 1 && ent_index <= 64) {
        LagRecord* rec = (LagRecord*)(records_base + (ent_index - 1) * 0x3234);
        rec->eye_angles.pitch = *pitch;

        // Anti-untrusted: clamp pitch
        if (*pitch > 89.f) *pitch = 89.f;
        if (*pitch < -89.f) *pitch = -89.f;
    }
}

void __cdecl ResolverEngine::OnYawProxy(float* yaw, int ent_index) {
    if (!yaw) return;

    uintptr_t records_base = 0;
    try {
        records_base = *(uintptr_t*)(0x43AF7A84);
    } catch (...) { return; }

    if (records_base && ent_index >= 1 && ent_index <= 64) {
        LagRecord* rec = (LagRecord*)(records_base + (ent_index - 1) * 0x3234);
        float original_yaw = *yaw;
        rec->eye_angles.yaw = original_yaw;

        // Apply resolver if enabled
        auto& resolver = Instance();
        if (resolver.enabled) {
            float resolved = ResolveYaw(ent_index, original_yaw, rec->lower_body_yaw);
            rec->eye_angles.yaw = resolved;
            // Optionally override the actual yaw that gets networked
            // *yaw = resolved; // This would be the resolver effect
        }
    }
}

void __cdecl ResolverEngine::OnLBYProxy(float* lby, int ent_index) {
    if (!lby) return;

    uintptr_t records_base = 0;
    try {
        records_base = *(uintptr_t*)(0x43AF7A84);
    } catch (...) { return; }

    if (records_base && ent_index >= 1 && ent_index <= 64) {
        LagRecord* rec = (LagRecord*)(records_base + (ent_index - 1) * 0x3234);
        float old_lby = rec->lower_body_yaw;
        rec->lower_body_yaw = *lby;

        // Update resolver info
        UpdatePlayerInfo(ent_index, *lby, rec->eye_angles.yaw, rec->velocity);

        // Detect LBY update
        if (std::abs(*lby - old_lby) > 0.1f) {
            LOG_INFO("LBY Update: player %d %.1f -> %.1f", ent_index, old_lby, *lby);
        }
    }
}

void __fastcall ResolverEngine::hkOnRenderStart(void* ecx, void* edx) {
    // Original at 0x34E25040: overrides render angles with resolved angles
    // Called during FrameStageNotify FRAME_RENDER_START

    // Get original function if available
    // For decompiled version, just log
    // Real implementation would correct thirdperson angles

    auto& ctx = GlobalContext::Instance();
    if (ctx.lagRecords) {
        // Iterate and fix render angles
        for (int i = 1; i <= 64; ++i) {
            LagRecord* rec = &ctx.lagRecords[i - 1];
            if (rec && rec->valid) {
                // Apply resolved angles to render
            }
        }
    }

    // Call original OnRenderStart if we have it
    uintptr_t original = *(uintptr_t*)(0x43AFE0E8);
    if (original && original != (uintptr_t)&hkOnRenderStart) {
        // Call original via detour
        // ((void(__thiscall*)(void*))original)(ecx);
    }
}

float ResolverEngine::ResolveYaw(int ent_index, float original_yaw, float lby) {
    auto& resolver = Instance();
    if (!resolver.enabled || ent_index < 1 || ent_index > 64) {
        return original_yaw;
    }

    PlayerResolveInfo* info = GetPlayerInfo(ent_index);
    if (!info) return original_yaw;

    switch (resolver.mode) {
        case ResolverMode::OFF:
            return original_yaw;

        case ResolverMode::LBY: {
            // Simple LBY resolver - use LBY when moving, last LBY when standing
            if (info->isMoving) {
                return lby;
            } else {
                // If LBY not updating, use last moving LBY or bruteforce
                if (info->isBreakingLBY) {
                    return info->lastMovingLBY;
                }
                return original_yaw;
            }
        }

        case ResolverMode::BRUTEFORCE: {
            // Bruteforce based on missed shots
            int stage = info->missedShots % 4;
            switch (stage) {
                case 0: return lby;
                case 1: return lby + 90.f;
                case 2: return lby - 90.f;
                case 3: return lby + 180.f;
                default: return lby;
            }
        }

        case ResolverMode::ANIM: {
            // Animation based - check desync delta
            float delta = std::abs(original_yaw - lby);
            if (delta > 35.f) {
                // Desync detected, try opposite
                return lby + (original_yaw > lby ? -120.f : 120.f);
            }
            return original_yaw;
        }

        case ResolverMode::ADVANCED: {
            // Advanced: combine LBY, movement, and bruteforce
            if (info->isMoving) {
                // When moving, LBY = real yaw
                info->lastMovingLBY = lby;
                return lby;
            }

            // Standing - check if breaking LBY
            float velLength = info->lastVelocity.Length2D();
            if (velLength < 0.1f) {
                // Standing
                float timeSinceUpdate = 0.f; // Would use curtime - lastUpdateTime
                if (timeSinceUpdate < 0.22f) {
                    // Just updated LBY, use it
                    return lby;
                } else {
                    // LBY not updating, bruteforce
                    int stage = info->missedShots % 5;
                    switch (stage) {
                        case 0: return info->lastMovingLBY;
                        case 1: return info->lastMovingLBY + 90.f;
                        case 2: return info->lastMovingLBY - 90.f;
                        case 3: return info->lastMovingLBY + 180.f;
                        case 4: return original_yaw + 180.f;
                        default: return info->lastMovingLBY;
                    }
                }
            }
            return original_yaw;
        }
    }

    return original_yaw;
}

void ResolverEngine::UpdatePlayerInfo(int ent_index, float lby, float yaw, const Vector& velocity) {
    if (ent_index < 1 || ent_index > 64) return;

    auto& resolver = Instance();
    PlayerResolveInfo& info = resolver.players_[ent_index];

    float velLen = velocity.Length2D();
    bool wasMoving = info.isMoving;
    info.isMoving = velLen > 0.1f;
    info.lastVelocity = velocity;

    // Detect LBY break
    if (std::abs(lby - info.lastLBY) > 35.f) {
        info.isBreakingLBY = true;
        info.lastUpdateTime = 0.f; // Would use curtime
        info.lbyDeltas.push_back(lby - info.lastLBY);
        if (info.lbyDeltas.size() > 10) info.lbyDeltas.pop_front();
    } else {
        info.isBreakingLBY = false;
    }

    if (info.isMoving) {
        info.lastMovingLBY = lby;
    }

    info.lastLBY = lby;
    info.simulationTime = 0.f; // Would use actual simtime
}

void ResolverEngine::OnMissedShot(int ent_index) {
    if (ent_index < 1 || ent_index > 64) return;
    auto& resolver = Instance();
    resolver.players_[ent_index].missedShots++;
    LOG_INFO("Resolver: missed shot on %d (total %d)", ent_index, resolver.players_[ent_index].missedShots);
}

void ResolverEngine::ResetPlayer(int ent_index) {
    if (ent_index < 1 || ent_index > 64) return;
    auto& resolver = Instance();
    resolver.players_[ent_index] = PlayerResolveInfo();
    resolver.players_[ent_index].index = ent_index;
}

PlayerResolveInfo* ResolverEngine::GetPlayerInfo(int ent_index) {
    if (ent_index < 1 || ent_index > 64) return nullptr;
    return &Instance().players_[ent_index];
}

} // namespace Aimware2016Decompiled
