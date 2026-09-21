#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include "../core/sdk.hpp"
#include "../core/logger.hpp"

namespace Aimware2016Decompiled {

using Aimware::SDK::Vector;
using Aimware::SDK::QAngle;
using Aimware::SDK::Matrix3x4;
using Aimware::SDK::CUserCmd;
using Aimware::SDK::LagRecord;
using Aimware::SDK::ProfileContext;
using Aimware::SDK::AwRender;
using Aimware::SDK::AwGlobals;
using Aimware::SDK::AwSkinChangerData;

// ===================== Enhanced Math =====================
namespace Math {

inline float Deg2Rad(float deg) { return deg * (3.14159265358979323846f / 180.f); }
inline float Rad2Deg(float rad) { return rad * (180.f / 3.14159265358979323846f); }

inline void SinCos(float rad, float* s, float* c) {
    *s = std::sin(rad);
    *c = std::cos(rad);
}

inline QAngle CalcAngle(const Vector& src, const Vector& dst) {
    Vector delta = dst - src;
    float hyp = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    QAngle angle;
    angle.pitch = Rad2Deg(std::atan2(-delta.z, hyp));
    angle.yaw = Rad2Deg(std::atan2(delta.y, delta.x));
    angle.roll = 0;
    angle.Clamp();
    return angle;
}

inline float GetFOV(const QAngle& view, const QAngle& aim) {
    Vector v1, v2;
    float pitch1 = Deg2Rad(view.pitch), yaw1 = Deg2Rad(view.yaw);
    float pitch2 = Deg2Rad(aim.pitch), yaw2 = Deg2Rad(aim.yaw);

    v1.x = std::cos(pitch1) * std::cos(yaw1);
    v1.y = std::cos(pitch1) * std::sin(yaw1);
    v1.z = -std::sin(pitch1);

    v2.x = std::cos(pitch2) * std::cos(yaw2);
    v2.y = std::cos(pitch2) * std::sin(yaw2);
    v2.z = -std::sin(pitch2);

    float dot = v1.Dot(v2);
    dot = std::clamp(dot, -1.f, 1.f);
    return Rad2Deg(std::acos(dot));
}

inline Vector AngleToVector(const QAngle& angle) {
    float sp, sy, cp, cy;
    SinCos(Deg2Rad(angle.pitch), &sp, &cp);
    SinCos(Deg2Rad(angle.yaw), &sy, &cy);
    return Vector(cp * cy, cp * sy, -sp);
}

inline void VectorAngles(const Vector& forward, QAngle& angles) {
    float tmp, yaw, pitch;
    if (forward.y == 0 && forward.x == 0) {
        yaw = 0;
        if (forward.z > 0) pitch = 270;
        else pitch = 90;
    } else {
        yaw = Rad2Deg(std::atan2(forward.y, forward.x));
        if (yaw < 0) yaw += 360;
        tmp = std::sqrt(forward.x * forward.x + forward.y * forward.y);
        pitch = Rad2Deg(std::atan2(-forward.z, tmp));
        if (pitch < 0) pitch += 360;
    }
    angles.pitch = pitch;
    angles.yaw = yaw;
    angles.roll = 0;
}

} // namespace Math

// ===================== Entity Interfaces (simplified) =====================
struct IClientEntity {
    virtual ~IClientEntity() {}
    // Common offsets - will be resolved via netvars
    Vector& GetOrigin() { return *(Vector*)((uintptr_t)this + 0x138); }
    Vector& GetVelocity() { return *(Vector*)((uintptr_t)this + 0x114); }
    int GetHealth() { return *(int*)((uintptr_t)this + 0xFC); }
    int GetTeam() { return *(int*)((uintptr_t)this + 0xF0); }
    bool IsDormant() { return *(bool*)((uintptr_t)this + 0xE9); }
    bool IsAlive() { return GetHealth() > 0; }
    int GetFlags() { return *(int*)((uintptr_t)this + 0x100); }
};

// ===================== Global Context =====================
struct GlobalContext {
    static GlobalContext& Instance() {
        static GlobalContext ctx;
        return ctx;
    }

    // Pointers to fixed memory
    AwRender* render = nullptr;
    AwGlobals* globals = nullptr;
    AwSkinChangerData* skinchanger = nullptr;
    LagRecord* lagRecords = nullptr; // base pointer

    // Interfaces
    void* engineClient = nullptr;
    void* entityList = nullptr;
    void* surface = nullptr;
    void* engineVGUI = nullptr;

    bool menuOpen = false;
    Vector localEyePos;

    void UpdateFromFixed() {
        render = *(AwRender**)(0x43B01224);
        globals = *(AwGlobals**)(0x43AF7704);
        skinchanger = *(AwSkinChangerData**)(0x43AF7700);
        lagRecords = (LagRecord*)*(uintptr_t*)(0x43AF7A84);

        if (render) {
            LOG_INFO("Render: %dx%d fontCreated=%d", render->Width, render->Height, render->DidCreateFont);
        }
    }
};

} // namespace Aimware2016Decompiled
