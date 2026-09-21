#pragma once
#include "decompiled_sdk.hpp"

namespace Aimware2016Decompiled
{
    // Decompiled Anti-Aim Subroutines from 0x34E2A000 - 0x34E2C000
    class AntiAimEngine
    {
    public:
        // VA: 0x34E2A910 - Pitch Anti-Aim Processor
        static float CalculatePitch(int mode);

        // VA: 0x34E2AF90 - Yaw Anti-Aim Processor
        static float CalculateYaw(int mode, CUserCmd* cmd, bool is_fake);

        // VA: 0x34E2B720 - FakeLag Choke Generator
        static void ProcessFakeLag(CUserCmd* cmd, bool& send_packet);
    };
}
