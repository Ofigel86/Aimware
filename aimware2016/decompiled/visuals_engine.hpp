#pragma once
#include "decompiled_sdk.hpp"

namespace Aimware2016Decompiled
{
    // Decompiled Visuals & Render Subroutines
    class VisualsEngine
    {
    public:
        // VA: 0x34E26660 - EngineVGUI::Paint Hook Handler
        static void __fastcall hkEngineVGUIPaint(void* ecx, void* edx, int mode);

        // VA: 0x34E25960 - StudioRender::DrawModel Chams Hook Handler
        static void __fastcall hkDrawModel(void* ecx, void* edx, void* results, void* info, Matrix3x4* bone_to_world, float* weights, float* origin, int flags);

        // VA: 0x34E26620 - Surface::LockCursor Mouse Unlock Handler
        static void __fastcall hkLockCursor(void* ecx, void* edx);

        // VA: 0x34E1D1D0 - Main ESP Drawing Loop
        static void DrawESP();
    };
}
