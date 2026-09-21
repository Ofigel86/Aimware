#include "config_ui_engine.hpp"
#include "../core/logger.hpp"
#include "../core/config_system.hpp"
#include <filesystem>
#include <fstream>

namespace Aimware2016Decompiled {

void __stdcall ConfigUIEngine::hkGenConfigPath(const char* name, wchar_t* out_path) {
    if (!name || !out_path) return;

    // Original at 0x34E34E90 - formats \??\C:\aimware\<name>
    // Improved version with validation

    // Sanitize name - remove invalid chars
    std::string safeName(name);
    safeName.erase(std::remove_if(safeName.begin(), safeName.end(), [](char c) {
        return c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|';
    }), safeName.end());

    // Use new config system for readable path
    Aimware::ConfigSystem::GenerateReadableConfigPath(safeName.c_str(), out_path);

    LOG_INFO("GenConfigPath: %s -> %ls", name, out_path);
}

bool ConfigUIEngine::DumpConfig(const char* config_name) {
    if (!config_name) return false;

    // Original at 0x34E357C5 - serializes binary config
    // Improved: save as readable JSON/INI + binary

    auto& engine = Instance();
    LOG_INFO("DumpConfig: %s with %zu values", config_name, engine.values_.size());

    // Build config data
    std::stringstream ss;
    ss << "; Aimware 2016 Config - " << config_name << "\n";
    ss << "; Generated: " << __DATE__ << " " << __TIME__ << "\n\n";

    std::string currentCategory;
    for (auto& [name, val] : engine.values_) {
        if (val.category != currentCategory) {
            currentCategory = val.category;
            ss << "[" << currentCategory << "]\n";
        }

        ss << name << "=";
        switch (val.type) {
            case ConfigValueType::BOOL: ss << (val.value.b ? "1" : "0"); break;
            case ConfigValueType::INT: ss << val.value.i; break;
            case ConfigValueType::FLOAT: ss << val.value.f; break;
            case ConfigValueType::COLOR: ss << val.value.color; break;
            default: ss << "0"; break;
        }
        ss << " ; " << val.description << "\n";
    }

    std::string data = ss.str();
    std::vector<uint8_t> bytes(data.begin(), data.end());

    return Aimware::ConfigSystem::Instance().SaveConfig(config_name, bytes);
}

std::vector<std::string> ConfigUIEngine::GetConfigList() {
    // Original at 0x34E35983 - scans C:\aimware\ for .cfg
    return Aimware::ConfigSystem::Instance().GetConfigList();
}

void ConfigUIEngine::RefreshConfigList() {
    Instance().configList_ = GetConfigList();
}

bool ConfigUIEngine::SaveConfig(const std::string& name) {
    return DumpConfig(name.c_str());
}

bool ConfigUIEngine::LoadConfig(const std::string& name) {
    LOG_INFO("LoadConfig: %s", name.c_str());

    try {
        std::filesystem::path fullPath = std::filesystem::path(Aimware::ConfigSystem::Instance().GetBasePath()) / name;
        if (!std::filesystem::exists(fullPath)) {
            LOG_ERROR("Config not found: %s", name.c_str());
            return false;
        }

        std::ifstream file(fullPath);
        std::string line;
        std::string currentCategory;

        auto& engine = Instance();
        while (std::getline(file, line)) {
            // Trim
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line[0] == ';' || line[0] == '#') continue;

            if (line[0] == '[' && line.back() == ']') {
                currentCategory = line.substr(1, line.size() - 2);
                continue;
            }

            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);

            // Remove comment after ;
            size_t comment = value.find(';');
            if (comment != std::string::npos) value = value.substr(0, comment);

            // Trim key and value
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Apply to config value
            ConfigValue* cfgVal = engine.GetValue(key);
            if (cfgVal) {
                try {
                    switch (cfgVal->type) {
                        case ConfigValueType::BOOL: cfgVal->value.b = (value == "1" || value == "true"); break;
                        case ConfigValueType::INT: cfgVal->value.i = std::stoi(value); break;
                        case ConfigValueType::FLOAT: cfgVal->value.f = std::stof(value); break;
                        default: break;
                    }
                } catch (...) {}
            }
        }

        LOG_SUCCESS("Config loaded: %s", name.c_str());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("LoadConfig failed: %s", e.what());
        return false;
    }
}

void ConfigUIEngine::ResetConfig() {
    auto& engine = Instance();
    for (auto& [name, val] : engine.values_) {
        switch (val.type) {
            case ConfigValueType::BOOL: val.value.b = false; break;
            case ConfigValueType::INT: val.value.i = (int)val.min; break;
            case ConfigValueType::FLOAT: val.value.f = val.min; break;
            default: break;
        }
    }
    LOG_INFO("Config reset to defaults");
}

void ConfigUIEngine::RegisterValue(const ConfigValue& val) {
    values_[val.name] = val;
}

ConfigValue* ConfigUIEngine::GetValue(const std::string& name) {
    auto it = values_.find(name);
    if (it != values_.end()) return &it->second;
    return nullptr;
}

// UI Rendering - simplified, would use ISurface
void ConfigUIEngine::DrawMenu() {
    auto& engine = Instance();
    if (!engine.menuOpen) return;

    // Menu rendering at original addresses:
    // DrawMenuHeader at 0x34E37D73, 0x34E3791E
    // DrawTabBar at 0x34E38FB2
    // DrawCheckbox at 0x34E372C5
    // etc

    // Simplified menu structure
    DrawTabBar();

    switch (engine.activeTab) {
        case 0: // Aimbot
            break;
        case 1: // Visuals
            break;
        case 2: // Misc
            break;
        case 3: // Skins
            break;
        case 4: // Config
            break;
    }
}

void ConfigUIEngine::DrawTabBar() {
    // Original at 0x34E38FB2
}

void ConfigUIEngine::DrawCheckbox(const char* name, bool* value) {
    // Original at 0x34E372C5
    if (!name || !value) return;
    // Would check mouse click in rect
}

void ConfigUIEngine::DrawSlider(const char* name, float* value, float min, float max, const char* fmt) {
    // Original at 0x34E37074
    if (!name || !value) return;
    *value = std::clamp(*value, min, max);
}

void ConfigUIEngine::DrawCombobox(const char* name, int* value, const std::vector<std::string>& items) {
    // Original at 0x34E388A4
}

void ConfigUIEngine::DrawButton(const char* name, std::function<void()> callback) {
    // Original at 0x34E39396
    if (callback) {
        // Check if clicked
        // callback();
    }
}

void ConfigUIEngine::DrawKeybind(const char* name, int* key) {
    // Original at 0x34E38185
}

void ConfigUIEngine::DrawColorPicker(const char* name, float* color) {
    // Would draw color picker
}

namespace UI {

bool IsMouseInRect(const Rect& r) {
    POINT p;
    GetCursorPos(&p);
    // Convert screen to client - would use ScreenToClient at 0x43AFF0CC
    return r.Contains(p.x, p.y);
}

void DrawRect(const Rect& r, int r_, int g_, int b_, int a_) {
    // Use ISurface at 0x43AFF098
    // surface->DrawOutlinedRect(x, y, x+w, y+h)
}

void DrawFilledRect(const Rect& r, int r_, int g_, int b_, int a_) {
    // surface->DrawFilledRect
}

void DrawText(int x, int y, const char* text, int r_, int g_, int b_) {
    // surface->DrawText
}

} // namespace UI

} // namespace Aimware2016Decompiled
