#pragma once
#include "decompiled_sdk.hpp"

namespace Aimware2016Decompiled
{
    // Decompiled Resolver & Proxy Subroutines from 0x34E24BF0 - 0x34E25040
    class ResolverEngine
    {
    public:
        // VA: 0x34E24BF0 - EyeAngles[0] Pitch Proxy Hook
        static void __cdecl OnPitchProxy(float* pitch, int ent_index);

        // VA: 0x34E24D20 - EyeAngles[1] Yaw Proxy Hook
        static void __cdecl OnYawProxy(float* yaw, int ent_index);

        // VA: 0x34E24EC0 - LowerBodyYawTarget Proxy Hook
        static void __cdecl OnLBYProxy(float* lby, int ent_index);

        // VA: 0x34E25040 - OnRenderStart View Angle Correction
        static void __fastcall hkOnRenderStart(void* ecx, void* edx);
    };
}
