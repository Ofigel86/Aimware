#include "antiaim_engine.hpp"

namespace Aimware2016Decompiled
{
    float AntiAimEngine::CalculatePitch(int mode)
    {
        // Assembly at 0x34E2A910: 89.0f (Emotion), -89.0f (Up), 0.0f (Zero), 180.0f (Fake)
        switch (mode)
        {
        case 1: return 89.0f;
        case 2: return -89.0f;
        case 3: return 0.0f;
        case 4: return 180.0f;
        default: return 0.0f;
        }
    }

    float AntiAimEngine::CalculateYaw(int mode, CUserCmd* cmd, bool is_fake)
    {
        // Assembly at 0x34E2AF90: Backward (180), Sideways (90), Spinbot, Jitter
        if (is_fake) return 90.0f;
        return 180.0f;
    }

    void AntiAimEngine::ProcessFakeLag(CUserCmd* cmd, bool& send_packet)
    {
        // Assembly at 0x34E2B720: Tick choking logic
        static int choked_ticks = 0;
        choked_ticks++;
        if (choked_ticks >= 6) { choked_ticks = 0; send_packet = true; }
        else send_packet = false;
    }
}
