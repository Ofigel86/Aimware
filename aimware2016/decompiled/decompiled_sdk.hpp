#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cmath>
#include <cstdint>
#include <vector>
#include <string>

namespace Aimware2016Decompiled
{
    struct Vector
    {
        float x, y, z;
        Vector() : x(0), y(0), z(0) {}
        Vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
        float Length() const { return std::sqrt(x*x + y*y + z*z); }
        float Length2D() const { return std::sqrt(x*x + y*y); }
    };

    struct QAngle
    {
        float pitch, yaw, roll;
        QAngle() : pitch(0), yaw(0), roll(0) {}
        QAngle(float p, float y, float r) : pitch(p), yaw(y), roll(r) {}
    };

    struct Matrix3x4
    {
        float m[3][4];
    };

    struct CUserCmd
    {
        void* vmt;
        int command_number;
        int tick_count;
        QAngle viewangles;
        Vector aimdirection;
        float forwardmove;
        float sidemove;
        float upmove;
        int buttons;
        uint8_t impulse;
        int weaponselect;
        int weaponsubtype;
        int random_seed;
        short mousedx;
        short mousedy;
        bool hasbeenpredicted;
    };

    // Lag Record structure allocated at 0x43AF7A84 (0x3234 bytes per player)
    struct LagRecord
    {
        Vector origin;
        Vector velocity;
        Vector mins;
        Vector maxs;
        QAngle eye_angles; // 0x3200 = pitch, 0x3204 = yaw
        float lower_body_yaw; // 0x320C
        float lby_update_time; // 0x3210
        float simulation_time;
        float old_simulation_time;
        int flags;
        Matrix3x4 bone_matrix[128];
        bool valid;
    };

    // Aimware Profile Context (0x43AFF218)
    struct profile_t
    {
        int xor_key;
        int pad;
        wchar_t config_path[260];
        int pad2[64];
    };
}
