#pragma once
#include "decompiled_sdk.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace Aimware2016Decompiled {

struct SkinInfo {
    int itemDefinitionIndex = 0;
    int paintKit = 0;
    int seed = 0;
    float wear = 0.0001f;
    int statTrak = -1;
    std::string customName;
    int accountId = 0;
};

struct GloveInfo {
    int itemDefinitionIndex = 0;
    int paintKit = 0;
    int modelIndex = 0;
    std::string modelPath;
};

class SkinChangerEngine {
public:
    static SkinChangerEngine& Instance() {
        static SkinChangerEngine inst;
        return inst;
    }

    // Original entry points
    static void OnPostDataUpdate(); // VA 0x34E2C580
    static void ApplyGloveModel();  // VA 0x34E1DFC9
    static void ApplyWeaponPaintkit(); // VA 0x34E1E2DA

    // Enhanced
    static void ApplySkins(void* entity);
    static void ApplyGlove(void* localPlayer);
    static void ForceModelUpdate();
    static void UpdateHud();

    static bool GetSkinForWeapon(int weaponId, SkinInfo& outSkin);
    static void SetSkinForWeapon(int weaponId, const SkinInfo& skin);

    static std::string DecryptString(const char* encrypted, int xorKey);
    static int GetModelIndex(const char* modelPath);

    bool enabled = true;
    std::unordered_map<int, SkinInfo> weaponSkins;
    GloveInfo gloveSkin;

private:
    SkinChangerEngine() = default;
    static void* GetItemSystem();
    static void* GetModelInfo();
};

} // namespace Aimware2016Decompiled
