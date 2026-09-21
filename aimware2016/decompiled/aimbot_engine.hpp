#pragma once
#include "decompiled_sdk.hpp"

namespace Aimware2016Decompiled
{
    // Decompiled Aimbot Subroutines from 0x34E28000 - 0x34E2A000
    class AimbotEngine
    {
    public:
        // VA: 0x34E258A0 - ClientMode::CreateMove Hook Handler
        static bool __stdcall hkCreateMove(float input_sample_frametime, CUserCmd* cmd);

        // VA: 0x34E282D0 - Hitchance Seed Calculator
        static bool CalculateHitchance(CUserCmd* cmd, const QAngle& angles, float required_hitchance);

        // VA: 0x34E28D40 - Best Target & FOV Scanner
        static int GetBestTarget(CUserCmd* cmd, Vector& out_target_pos);

        // VA: 0x34E29040 - AutoWall Penetration Simulator
        static float SimulateAutoWall(void* local_player, const Vector& target_pos);

        // VA: 0x34E29B90 - Multipoint Hitbox Point Generator
        static void ScanHitboxPoints(void* entity, int hitbox_index, std::vector<Vector>& out_points);
    };
}
