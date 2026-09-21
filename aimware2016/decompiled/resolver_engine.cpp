#include "resolver_engine.hpp"

namespace Aimware2016Decompiled
{
    void __cdecl ResolverEngine::OnPitchProxy(float* pitch, int ent_index)
    {
        // Assembly at 0x34E24BF0: Stores pitch at offset 0x3200 in lag records (0x43AF7A84)
        if (!pitch) return;
        uintptr_t records_base = *(uintptr_t*)(0x43AF7A84);
        if (records_base && ent_index >= 1 && ent_index <= 64)
        {
            LagRecord* rec = (LagRecord*)(records_base + (ent_index - 1) * 0x3234);
            rec->eye_angles.pitch = *pitch;
        }
    }

    void __cdecl ResolverEngine::OnYawProxy(float* yaw, int ent_index)
    {
        // Assembly at 0x34E24D20: Stores yaw at offset 0x3204 in lag records
        if (!yaw) return;
        uintptr_t records_base = *(uintptr_t*)(0x43AF7A84);
        if (records_base && ent_index >= 1 && ent_index <= 64)
        {
            LagRecord* rec = (LagRecord*)(records_base + (ent_index - 1) * 0x3234);
            rec->eye_angles.yaw = *yaw;
        }
    }

    void __cdecl ResolverEngine::OnLBYProxy(float* lby, int ent_index)
    {
        // Assembly at 0x34E24EC0: Stores LBY at offset 0x320C in lag records
        if (!lby) return;
        uintptr_t records_base = *(uintptr_t*)(0x43AF7A84);
        if (records_base && ent_index >= 1 && ent_index <= 64)
        {
            LagRecord* rec = (LagRecord*)(records_base + (ent_index - 1) * 0x3234);
            rec->lower_body_yaw = *lby;
        }
    }

    void __fastcall ResolverEngine::hkOnRenderStart(void* ecx, void* edx)
    {
        // Assembly at 0x34E25040: Overrides render angles with resolved angles
    }
}
