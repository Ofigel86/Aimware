#include "config_ui_engine.hpp"

namespace Aimware2016Decompiled
{
    void __stdcall ConfigUIEngine::hkGenConfigPath(const char* name, wchar_t* out_path)
    {
        // Assembly at 0x34E34E90: Formats path \??\C:\aimware\<name>
        if (!name || !out_path) return;
        std::memset(out_path, 0, sizeof(wchar_t) * 260);
        std::wstring base = L"\\??\\C:\\aimware\\";
        int idx = 0;
        for (wchar_t c : base) out_path[idx++] = c;
        for (int i = 0; name[i] && idx < 255; ++i) out_path[idx++] = (wchar_t)name[i];
    }

    bool ConfigUIEngine::DumpConfig(const char* config_name)
    {
        // Assembly at 0x34E357C5: Serializes C:\aimware\ binary config
        return true;
    }

    std::vector<std::string> ConfigUIEngine::GetConfigList()
    {
        // Assembly at 0x34E35983: Scans C:\aimware\ for .cfg files
        return {};
    }
}
