#pragma once
#include "decompiled_sdk.hpp"

namespace Aimware2016Decompiled
{
    // Decompiled SkinChanger Subroutines
    class SkinChangerEngine
    {
    public:
        // VA: 0x34E2C580 - FrameStageNotify Skin Update Routine
        static void OnPostDataUpdate();

        // VA: 0x34E1DFC9 - GloveChanger Model Loader
        static void ApplyGloveModel();

        // VA: 0x34E1E2DA - PaintkitChanger Override
        static void ApplyWeaponPaintkit();
    };
}
