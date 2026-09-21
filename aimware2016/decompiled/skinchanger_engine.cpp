#include "skinchanger_engine.hpp"

namespace Aimware2016Decompiled
{
    void SkinChangerEngine::OnPostDataUpdate()
    {
        // Assembly at 0x34E2C580: PostDataUpdate sequence
        ApplyGloveModel();
        ApplyWeaponPaintkit();
    }

    void SkinChangerEngine::ApplyGloveModel()
    {
        // Assembly at 0x34E1DFC9: Decrypt string keys from 0x76ED9CE0 using XOR key in profile_t (0x43AFF218)
    }

    void SkinChangerEngine::ApplyWeaponPaintkit()
    {
        // Assembly at 0x34E1E2DA: Parse paintkit ID from config and override m_nFallbackPaintKit (0x43AFEF04)
    }
}
