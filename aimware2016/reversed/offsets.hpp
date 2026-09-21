#pragma once
#include <cstdint>

/*
 * Full Reverse Engineering: CS:GO 2016/2018 Netvar Offsets
 * 
 * These offsets are for client.dll and are resolved via netvars.
 * The cheat stores them at fixed addresses in data section.
 */

namespace Aimware::Reversed::Offsets {

// ===================== Netvar Storage (0x43AFxxxx) =====================
namespace NetvarStorage {
    constexpr uintptr_t M_YAW = 0x43AFEFE4;
    constexpr uintptr_t M_PITCH = 0x43AFEFE4;
    constexpr uintptr_t SENSITIVITY = 0x43AFEFD4;
    constexpr uintptr_t SV_CHEATS = 0x43AFEFF0;
    constexpr uintptr_t SV_FOOTSTEPS = 0x43AFEFF8;
    constexpr uintptr_t CL_MODELFASTPATH = 0x43AFF008;
    constexpr uintptr_t WEAPON_RECOIL_SCALE = 0x43AFF02C;
    constexpr uintptr_t SV_GRAVITY = 0x43AFF07C;
    constexpr uintptr_t CL_INTERPOLATE = 0x43AFF09C;
    constexpr uintptr_t VIEW_RECOIL_TRACKING = 0x43AFF0B8;
    constexpr uintptr_t SV_MAXUPDATERATE = 0x43AFF05C;
    constexpr uintptr_t CL_INTERP = 0x43AFF060;
    constexpr uintptr_t SV_MINUPDATERATE = 0x43AFF064;
    constexpr uintptr_t CL_INTERP_RATIO = 0x43AFF068;
    constexpr uintptr_t SV_CLIENT_MAX_INTERP_RATIO = 0x43AFF070;
    constexpr uintptr_t CL_UPDATERATE = 0x43AFF074;
    constexpr uintptr_t SV_CLIENT_MIN_INTERP_RATIO = 0x43AFF078;
    constexpr uintptr_t MOLOTOV_THROW_DETONATE = 0x43AFF080;
    constexpr uintptr_t MOLOTOV_MAX_SLOPE = 0x43AFF084;
    constexpr uintptr_t NAME = 0x43AFF00C;
    constexpr uintptr_t R_3DSKY = 0x43AFEFE0;
    constexpr uintptr_t R_DRAWSKYBOX = 0x43AFF010;
    constexpr uintptr_t GL_CLEAR = 0x43AFF014;
    constexpr uintptr_t CL_FULLUPDATE = 0x43AFF058;
}

// ===================== Player Offsets (CCSPlayer) =====================
namespace CCSPlayer {
    // These are resolved at runtime via netvars, but we document their storage
    constexpr uintptr_t M_FL_NEXT_ATTACK = 0x43AFEEA0; // CBaseCombatCharacter
    constexpr uintptr_t M_H_ACTIVE_WEAPON = 0x43AFEEA8;
    constexpr uintptr_t M_H_MY_WEAPONS = 0x43AFEEAC; // Actually m_hMyWearables
    constexpr uintptr_t M_FL_POSE_PARAMETER = 0x43AFEEB4;
    constexpr uintptr_t M_B_CLIENT_SIDE_ANIM = 0x43AFEEBC;
    constexpr uintptr_t M_B_HAS_HELMET = 0x43AFEF70;
    constexpr uintptr_t M_B_HAS_DEFUSER = 0x43AFEF74;
    constexpr uintptr_t M_I_ACCOUNT = 0x43AFEF78;
    constexpr uintptr_t M_B_IS_DEFUSING = 0x43AFEF7C;
    constexpr uintptr_t M_FL_LOWER_BODY_YAW = 0x43AFEF80;
    constexpr uintptr_t M_I_SHOTS_FIRED = 0x43AFEF84;
    constexpr uintptr_t M_B_GUN_GAME_IMMUNITY = 0x43AFEF88;
    constexpr uintptr_t M_AIM_PUNCH_ANGLE = 0x43AFEE9C;
    constexpr uintptr_t M_ARMOR_VALUE = 0x43AFEF6C; // CBasePlayer
    constexpr uintptr_t M_N_TICK_BASE = 0x43AFEEE4;
    constexpr uintptr_t M_DEADFLAG = 0x43AFEEE0;
    constexpr uintptr_t M_F_FLAGS = 0x43AFEF34; // Actually stored here
    constexpr uintptr_t M_ANG_EYE_ANGLES = 0x43AFEFB8; // m_angEyeAngles[0]
    constexpr uintptr_t M_ANG_EYE_ANGLES_1 = 0x43AFEF34; // Actually m_angEyeAngles[1] proxy
    constexpr uintptr_t M_FL_FLASH_DURATION = 0x43AFEFC8;
    constexpr uintptr_t M_FL_THIRDPERSON_RECOIL = 0x43AFEED8;
}

// ===================== Weapon Offsets (CBaseCombatWeapon) =====================
namespace CBaseCombatWeapon {
    constexpr uintptr_t M_I_ITEM_DEFINITION_INDEX = 0x43AFEEF8;
    constexpr uintptr_t M_I_CLIP1 = 0x43AFEEFC;
    constexpr uintptr_t M_I_CLIP2 = 0x43AFEF38;
    constexpr uintptr_t M_I_ACCOUNT_ID = 0x43AFEF40;
    constexpr uintptr_t M_I_VIEW_MODEL_INDEX = 0x43AFEF20;
    constexpr uintptr_t M_I_WORLD_MODEL_INDEX = 0x43AFEF24;
    constexpr uintptr_t M_I_PRIMARY_RESERVE_AMMO = 0x43AFEF30;
    constexpr uintptr_t M_FL_POSTPONE_FIRE_READY = 0x43AFEFB4;
    constexpr uintptr_t M_FL_NEXT_PRIMARY_ATTACK = 0x43AFEEF4; // 0x31D8
}

// ===================== Attributable Item (CBaseAttributableItem) =====================
namespace CBaseAttributableItem {
    constexpr uintptr_t M_N_FALLBACK_STAT_TRACK = 0x43AFEF00;
    constexpr uintptr_t M_N_FALLBACK_PAINT_KIT = 0x43AFEF04;
    constexpr uintptr_t M_ORIGINAL_OWNER_XUID_LOW = 0x43AFEF08;
    constexpr uintptr_t M_B_INITIALIZED = 0x43AFEF0C;
    constexpr uintptr_t M_SZ_CUSTOM_NAME = 0x43AFEF10;
    constexpr uintptr_t M_I_ITEM_ID_LOW = 0x43AFEF14;
    constexpr uintptr_t M_N_FALLBACK_SEED = 0x43AFEF28;
    constexpr uintptr_t M_FL_FALLBACK_WEAR = 0x43AFEF2C;
}

// ===================== CS:GO 2016 vs 2018 Differences =====================
namespace VersionDifferences {
    // 2016 (original)
    namespace CSGO2016 {
        constexpr uintptr_t GLOBAL_VARS_INDEX = 0x53; // client vtable[0] + 0x53
        constexpr uintptr_t GET_CS_WPN_DATA_PATTERN = 0x43AFEF3C; // 66 3B 0D...
        constexpr uintptr_t HUD_PATTERN = 0x43AFEFA4; // B9 ? ? ? ? 56 68...
        constexpr uintptr_t PRED_SEED_PATTERN = 0x43AFEE5C; // 8B 0D ? ? ? ? BA...
        constexpr uintptr_t GLOW_PATTERN = 0x43AFEFC4; // A1 ? ? ? ? A8 ? 75...
    }

    // 2018 (updated)
    namespace CSGO2018 {
        constexpr uintptr_t GLOBAL_VARS_INDEX = 0x1B; // client vtable[0] + 0x1B
        constexpr uintptr_t GET_CS_WPN_DATA_PATTERN = 0x43AFEF3C; // 55 8B EC 81 EC...
        constexpr uintptr_t HUD_PATTERN = 0x43AFEFA4; // B9 ? ? ? ? 0F 94...
        constexpr uintptr_t PRED_SEED_PATTERN = 0x43AFEE5C; // C7 05 ? ? ? ? ? ? ? ? EB...
        constexpr uintptr_t GLOW_PATTERN = 0x43AFEFC4; // A1 ? ? ? ? A8 01 75 4B

        // Skinchanger offsets changed in 2018
        constexpr int SKINCHANGER_OFFSET_DELTA = 0x24;
        // 0xF0 -> 0xF0+0x24, etc
        constexpr uintptr_t SKIN_0xF0 = 0x34E1D948;
        constexpr uintptr_t SKIN_0x284 = 0x34E1D954;
        constexpr uintptr_t SKIN_0x218 = 0x34E1D960;
        constexpr uintptr_t SKIN_0xD8 = 0x34E1D9A2;

        // Weapon vtable indexes changed
        constexpr int GET_SPREAD_INDEX = 469;
        constexpr int GET_INACCURACY_INDEX = 471;
        constexpr int UPDATE_ACCURACY_INDEX = 439;
    }
}

// ===================== Lag Record Layout (0x43AF7A84) =====================
namespace LagRecord {
    constexpr size_t SIZE = 0x3234;
    constexpr size_t MAX_PLAYERS = 64;

    // Offsets within LagRecord
    constexpr size_t ORIGIN = 0x0; // Vector
    constexpr size_t VELOCITY = 0xC; // Vector
    constexpr size_t MINS = 0x18; // Vector
    constexpr size_t MAXS = 0x24; // Vector
    constexpr size_t EYE_ANGLES_PITCH = 0x3200; // float
    constexpr size_t EYE_ANGLES_YAW = 0x3204; // float
    constexpr size_t EYE_ANGLES_ROLL = 0x3208; // float
    constexpr size_t LOWER_BODY_YAW = 0x320C; // float
    constexpr size_t LBY_UPDATE_TIME = 0x3210; // float
    constexpr size_t SIMULATION_TIME = 0x3214; // float
    constexpr size_t OLD_SIM_TIME = 0x3218; // float
    constexpr size_t FLAGS = 0x321C; // int
    constexpr size_t BONE_MATRIX = 0x3220; // Matrix3x4[128] starts here? Actually earlier
    constexpr size_t VALID = 0x3220 + 128 * 48; // bool

    // Calculation: records_base + ent_index * 0x3234
    inline uintptr_t GetRecordAddress(uintptr_t base, int entIndex) {
        return base + entIndex * SIZE;
    }
}

} // namespace Aimware::Reversed::Offsets
