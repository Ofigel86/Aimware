#include "skinchanger_engine.hpp"
#include "../core/logger.hpp"

namespace Aimware2016Decompiled {

void SkinChangerEngine::OnPostDataUpdate() {
    // Called at FRAME_NET_UPDATE_POSTDATAUPDATE_START (stage 4)
    // Original at 0x34E2C580

    auto& changer = Instance();
    if (!changer.enabled) return;

    ApplyGloveModel();
    ApplyWeaponPaintkit();

    LOG_INFO("SkinChanger: OnPostDataUpdate executed");
}

void SkinChangerEngine::ApplyGloveModel() {
    // Original at 0x34E1DFC9 - decrypt glove model path using XOR key

    uintptr_t profile = *(uintptr_t*)(0x43AFF218);
    if (!profile) {
        LOG_WARN("SkinChanger: no profile context");
        return;
    }

    int xorKey = *(int*)profile;
    const char* encryptedPath = (const char*)(0x76ED9CE0); // Encrypted glove model path

    // Decrypt
    std::string decrypted = DecryptString(encryptedPath, xorKey);
    LOG_INFO("SkinChanger: decrypted glove model: %s (key %d)", decrypted.c_str(), xorKey);

    // Get local player and apply glove
    void* entityList = *(void**)(0x43AFEFDC);
    void* engineClient = *(void**)(0x43AFF050);
    if (!entityList || !engineClient) return;

    // Get local player index via engine client vtable[12]
    // int localIndex = ((int(__thiscall*)(void*))(*(uintptr_t**)engineClient)[12])(engineClient);
    // void* localPlayer = GetClientEntity(localIndex);

    // Apply glove model override
    // Would set m_hMyWearables and model index
}

void SkinChangerEngine::ApplyWeaponPaintkit() {
    // Original at 0x34E1E2DA - parse paintkit ID from config

    auto& changer = Instance();
    if (!changer.enabled) return;

    // Get item system at 0x43AFEFD0
    void* itemSystem = *(void**)(0x43AFEFD0);
    if (!itemSystem) return;

    // Iterate weapons and apply paintkits
    // For each weapon entity, set:
    // m_nFallbackPaintKit (0x43AFEF04)
    // m_flFallbackWear (0x43AFEF2C)
    // m_nFallbackSeed (0x43AFEF28)
    // m_nFallbackStatTrak (0x43AFEF00)
    // m_szCustomName (0x43AFEF10)

    LOG_INFO("SkinChanger: ApplyWeaponPaintkit - %zu skins configured", changer.weaponSkins.size());

    // Example: apply to all weapons
    for (auto& [weaponId, skin] : changer.weaponSkins) {
        // Find weapon entity and override
        // *(int*)((uintptr_t)weaponEnt + m_nFallbackPaintKit) = skin.paintKit;
        // etc
    }
}

void SkinChangerEngine::ApplySkins(void* entity) {
    if (!entity) return;

    auto& changer = Instance();
    if (!changer.enabled) return;

    // Get weapon ID
    // int weaponId = *(int*)((uintptr_t)entity + m_iItemDefinitionIndex);
    // SkinInfo skin;
    // if (GetSkinForWeapon(weaponId, skin)) { apply }
}

void SkinChangerEngine::ApplyGlove(void* localPlayer) {
    if (!localPlayer) return;

    auto& changer = Instance();
    if (changer.gloveSkin.modelPath.empty()) return;

    // Get wearable handle at m_hMyWearables (0x43AFEEAC)
    // Create glove entity if not exists
    // Set model index via MDLCache at 0x43AFF0B0
}

void SkinChangerEngine::ForceModelUpdate() {
    // Force full update via cl_fullupdate command at 0x43AFF058
    void* fullUpdateCmd = *(void**)(0x43AFF058);
    if (fullUpdateCmd) {
        // Call ConCommand execution
        LOG_INFO("SkinChanger: forcing model update via cl_fullupdate");
    }

    // Also set m_bInitialized to false to force re-init
}

void SkinChangerEngine::UpdateHud() {
    // Find HUD element via FindHudElement at 0x43AFEFA0
    uintptr_t findHudElement = *(uintptr_t*)(0x43AFEFA0);
    if (!findHudElement) return;

    // Get pHud at 0x43AFEFA4
    // Find CCSGO_HudWeaponSelection or similar and update
}

bool SkinChangerEngine::GetSkinForWeapon(int weaponId, SkinInfo& outSkin) {
    auto& changer = Instance();
    auto it = changer.weaponSkins.find(weaponId);
    if (it != changer.weaponSkins.end()) {
        outSkin = it->second;
        return true;
    }
    return false;
}

void SkinChangerEngine::SetSkinForWeapon(int weaponId, const SkinInfo& skin) {
    Instance().weaponSkins[weaponId] = skin;
    LOG_SUCCESS("Skin set for weapon %d -> paintkit %d", weaponId, skin.paintKit);
}

std::string SkinChangerEngine::DecryptString(const char* encrypted, int xorKey) {
    if (!encrypted) return "";
    if (xorKey == 0) return std::string(encrypted); // No encryption if key 0

    std::string result;
    for (int i = 0; encrypted[i] != '\0'; ++i) {
        char decrypted = encrypted[i] ^ (char)(xorKey & 0xFF);
        // Also XOR with position for stronger obfuscation
        // decrypted ^= (char)(i & 0xFF);
        if (decrypted == '\0') break;
        result += decrypted;
    }
    return result;
}

int SkinChangerEngine::GetModelIndex(const char* modelPath) {
    if (!modelPath) return 0;

    // Use MDLCache at 0x43AFF0B0
    void* mdlCache = *(void**)(0x43AFF0B0);
    if (!mdlCache) return 0;

    // Call FindMDL - signature at MDLCache vtable
    // int index = ((int(__thiscall*)(void*, const char*))(*(uintptr_t**)mdlCache)[10])(mdlCache, modelPath);
    // return index;

    LOG_INFO("GetModelIndex: %s", modelPath);
    return 0;
}

void* SkinChangerEngine::GetItemSystem() {
    return *(void**)(0x43AFEFD0);
}

void* SkinChangerEngine::GetModelInfo() {
    return *(void**)(0x43AFF04C);
}

} // namespace Aimware2016Decompiled
