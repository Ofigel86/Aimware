#pragma once
#include "decompiled_sdk.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace Aimware2016Decompiled {

enum class ConfigValueType {
    BOOL,
    INT,
    FLOAT,
    COLOR,
    STRING,
    KEYBIND
};

struct ConfigValue {
    ConfigValueType type;
    std::string name;
    std::string category;
    union {
        bool b;
        int i;
        float f;
        int color; // RGBA
    } value;
    float min, max;
    std::string description;

    ConfigValue() : type(ConfigValueType::BOOL), min(0), max(1) { value.b = false; }
};

class ConfigUIEngine {
public:
    static ConfigUIEngine& Instance() {
        static ConfigUIEngine inst;
        return inst;
    }

    // Original handlers
    static void __stdcall hkGenConfigPath(const char* name, wchar_t* out_path);
    static bool DumpConfig(const char* config_name);
    static std::vector<std::string> GetConfigList();

    // Enhanced UI
    static void DrawMenu();
    static void DrawTabBar();
    static void DrawCheckbox(const char* name, bool* value);
    static void DrawSlider(const char* name, float* value, float min, float max, const char* fmt = "%.1f");
    static void DrawCombobox(const char* name, int* value, const std::vector<std::string>& items);
    static void DrawButton(const char* name, std::function<void()> callback);
    static void DrawKeybind(const char* name, int* key);
    static void DrawColorPicker(const char* name, float* color);

    // Config management
    static bool SaveConfig(const std::string& name);
    static bool LoadConfig(const std::string& name);
    static void ResetConfig();

    void RegisterValue(const ConfigValue& val);
    ConfigValue* GetValue(const std::string& name);

    bool menuOpen = false;
    int activeTab = 0;

private:
    ConfigUIEngine() = default;
    std::unordered_map<std::string, ConfigValue> values_;
    std::vector<std::string> configList_;

    static void RefreshConfigList();
};

// UI Helpers
namespace UI {
    struct Rect {
        int x, y, w, h;
        bool Contains(int px, int py) const {
            return px >= x && px <= x + w && py >= y && py <= y + h;
        }
    };

    bool IsMouseInRect(const Rect& r);
    void DrawRect(const Rect& r, int r_, int g_, int b_, int a_ = 255);
    void DrawFilledRect(const Rect& r, int r_, int g_, int b_, int a_ = 255);
    void DrawText(int x, int y, const char* text, int r_ = 255, int g_ = 255, int b_ = 255);
}

} // namespace Aimware2016Decompiled
