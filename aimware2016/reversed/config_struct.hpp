#pragma once
#include <cstdint>
#include <array>

/*
 * Full Reverse Engineering: Aimware 2016 Config Structure
 * 
 * Config is stored at C:\aimware\ as binary files.
 * Format: XOR encrypted with profile key, then serialized structs.
 */

namespace Aimware::Reversed::Config {

// Config file header
struct ConfigHeader {
    uint32_t magic; // 'AWC0' or similar
    uint32_t version;
    uint32_t checksum;
    uint32_t size;
    char name[32];
};

// Aimbot config (from 0x43AFBEA2 and surrounding)
struct AimbotConfig {
    bool enabled; // 0x43AFBEA2
    bool silent;
    bool autoShoot;
    bool autoScope;
    bool autoStop;
    bool autoCrouch;
    bool smokeCheck;
    bool flashCheck;
    bool jumpCheck;
    bool autoWall;
    bool multipoint;
    bool hitchanceEnabled;
    float hitchance; // 0-100
    float fov; // 0-180
    float minDamage;
    int hitbox; // 0=head, 1=chest, etc
    int multipointScale; // 0-100
    bool backtrack;
    int backtrackTicks;
    char pad[64];
};

// Anti-aim config
struct AntiAimConfig {
    bool enabled;
    int pitchMode; // 0=none, 1=emotion(89), 2=up(-89), 3=zero, 4=fake(180)
    int yawMode; // 0=none, 1=backward(180), 2=sideways(90), 3=spinbot, 4=jitter
    int fakeYawMode;
    bool atTargets;
    bool edge;
    bool knifeHeld;
    bool fakeLagEnabled;
    int fakeLag; // 1-14
    float spinSpeed;
    float jitterRange;
    bool lbyBreaker;
    char pad[64];
};

// ESP config
struct ESPConfig {
    bool enabled;
    bool box;
    int boxType; // 0=none, 1=2D, 2=3D, 3=corners
    bool healthBar;
    bool armorBar;
    bool name;
    bool weapon;
    bool ammo;
    bool skeleton;
    bool snapLines;
    bool glow;
    bool chams;
    bool chamsIgnoreZ;
    int chamsMaterial; // 0=flat, 1=textured, 2=metallic
    bool chamsVisibleOnly;
    float chamsHiddenColor[4];
    float chamsVisibleColor[4];
    bool noSmoke;
    bool noFlash;
    bool noRecoil;
    bool noScope;
    bool thirdPerson;
    float thirdPersonDistance;
    bool radar;
    bool soundESP;
    bool grenadePrediction;
    char pad[128];
};

// Skinchanger config
struct SkinchangerConfig {
    bool enabled;
    struct WeaponSkin {
        int definitionIndex;
        int paintKit;
        int seed;
        float wear;
        int statTrak;
        char customName[32];
        bool enabled;
    };
    std::array<WeaponSkin, 64> weapons;
    struct GloveSkin {
        int definitionIndex;
        int paintKit;
        char modelPath[128];
        bool enabled;
    } glove;
    char pad[256];
};

// Misc config
struct MiscConfig {
    bool bunnyHop;
    bool autoStrafe;
    bool chatSpam;
    bool nameSpam;
    char chatSpamText[128];
    bool nameStealer;
    bool clanTag;
    char clanTagText[32];
    bool rankReveal;
    bool noFlash;
    bool noSmoke;
    bool thirdPerson;
    int thirdPersonKey;
    bool hitSound;
    int hitSoundType;
    bool hitMarker;
    char pad[128];
};

// Full config
struct FullConfig {
    ConfigHeader header;
    AimbotConfig aimbot;
    AntiAimConfig antiaim;
    ESPConfig esp;
    SkinchangerConfig skinchanger;
    MiscConfig misc;
    // Keybinds
    struct Keybind {
        int key;
        int mode; // 0=always, 1=hold, 2=toggle
        bool enabled;
    };
    std::array<Keybind, 32> keybinds;
    char pad[1024];
};

// Config file handling (from 0x34E34746, 0x34E357C5, 0x34E35983)
namespace Functions {
    // Original addresses
    constexpr uintptr_t CONFIG_ACTION = 0x34E34746; // Handles save/load/delete
    constexpr uintptr_t DUMP_CONFIG = 0x34E357C5; // Serializes config to file
    constexpr uintptr_t GET_CONFIG_LIST = 0x34E35983; // Scans C:\aimware\ for configs
    constexpr uintptr_t PREPARE_CONFIG_SAVE = 0x34E1EAAD; // Prepares data for save
    constexpr uintptr_t GEN_CONFIG_PATH = 0x34E34E90; // Generates \??\C:\aimware\<name>

    // Config path generation
    // Original: formats \??\C:\aimware\<name>
    // \??\ is NT path prefix
    inline void GenConfigPath(const char* name, wchar_t* out) {
        if (!name || !out) return;
        const wchar_t* base = L"\\??\\C:\\aimware\\";
        int idx = 0;
        for (int i = 0; base[i]; ++i) out[idx++] = base[i];
        for (int i = 0; name[i] && idx < 255; ++i) out[idx++] = (wchar_t)name[i];
        out[idx] = 0;
    }

    // Config list scanning
    // Original uses FindFirstFileW, FindNextFileW, FindClose
    // Scans C:\aimware\ for *.cfg or similar
}

// Profile context (0x43AFF218)
struct ProfileContext {
    int xor_key; // Used to decrypt strings
    int pad;
    wchar_t config_path[260]; // L"\\??\\C:\\aimware\\"
    int pad2[64];
    // More fields...
};

} // namespace Aimware::Reversed::Config
