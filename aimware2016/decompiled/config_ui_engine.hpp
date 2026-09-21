#pragma once
#include "decompiled_sdk.hpp"

namespace Aimware2016Decompiled
{
    // Decompiled Config & GUI Subroutines
    class ConfigUIEngine
    {
    public:
        // VA: 0x34E34E90 - hkGenConfigPath Handler (C:\aimware\)
        static void __stdcall hkGenConfigPath(const char* name, wchar_t* out_path);

        // VA: 0x34E357C5 - DumpConfig Serializer
        static bool DumpConfig(const char* config_name);

        // VA: 0x34E35983 - GetConfigList Scanner
        static std::vector<std::string> GetConfigList();
    };
}
