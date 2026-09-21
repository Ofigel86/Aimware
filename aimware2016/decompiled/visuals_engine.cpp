#include "visuals_engine.hpp"

namespace Aimware2016Decompiled
{
    void __fastcall VisualsEngine::hkEngineVGUIPaint(void* ecx, void* edx, int mode)
    {
        // Assembly at 0x34E26660: Calls original Paint, checks PAINT_UIPANELS (bit 0), calls DrawESP (0x34E1D1D0)
        if (mode & 1) DrawESP();
    }

    void __fastcall VisualsEngine::hkDrawModel(void* ecx, void* edx, void* results, void* info, Matrix3x4* bone_to_world, float* weights, float* origin, int flags)
    {
        // Assembly at 0x34E25960: Chams material application from 0x76ED9EA0
    }

    void __fastcall VisualsEngine::hkLockCursor(void* ecx, void* edx)
    {
        // Assembly at 0x34E26620: UnlockCursor when menu is open (0x43AFD094 == 1)
        bool menu_open = *(uint8_t*)(0x43AFD094) != 0;
        if (menu_open)
        {
            // Unlock mouse cursor
        }
    }

    void VisualsEngine::DrawESP()
    {
        // Assembly at 0x34E1D1D0: Iterate 1..64 entities and draw 2D/3D Bounding Box, Health bar, Bones, Name
    }
}
