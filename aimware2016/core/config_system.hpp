#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "logger.hpp"

namespace Aimware {

class ConfigSystem {
public:
    static ConfigSystem& Instance() {
        static ConfigSystem instance;
        return instance;
    }

    bool Initialize(const std::wstring& base_path = L"C:\\aimware\\") {
        base_path_ = base_path;
        try {
            std::filesystem::create_directories(base_path_);
            LOG_SUCCESS("Config directory ensured: %ls", base_path_.c_str());
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create config directory: %s", e.what());
            return false;
        }
    }

    std::vector<std::string> GetConfigList() {
        std::vector<std::string> configs;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(base_path_)) {
                if (entry.is_regular_file()) {
                    auto ext = entry.path().extension();
                    if (ext == L".cfg" || ext == L".ini" || ext == L".dat") {
                        configs.push_back(entry.path().filename().string());
                    }
                }
            }
        } catch (...) {
            LOG_ERROR("Failed to scan config directory");
        }
        LOG_INFO("Found %zu configs", configs.size());
        return configs;
    }

    static void GenerateConfigPath(const char* name, wchar_t* out) {
        if (!name || !out) return;
        memset(out, 0, sizeof(wchar_t) * 260);
        std::wstring base = L"\\??\\C:\\aimware\\";
        int idx = 0;
        for (wchar_t c : base) out[idx++] = c;
        for (int i = 0; name[i] && idx < 255; ++i) out[idx++] = (wchar_t)name[i];
    }

    static void GenerateReadableConfigPath(const char* name, wchar_t* out) {
        if (!name || !out) return;
        memset(out, 0, sizeof(wchar_t) * 64);
        size_t len = strlen(name);
        for (size_t i = 0; i < len && i < 63; ++i) {
            out[i] = (wchar_t)name[i];
        }
    }

    bool SaveConfig(const std::string& name, const std::vector<uint8_t>& data) {
        try {
            std::filesystem::path full_path = std::filesystem::path(base_path_) / name;
            std::ofstream file(full_path, std::ios::binary);
            if (!file) return false;
            file.write((const char*)data.data(), data.size());
            LOG_SUCCESS("Config saved: %s", name.c_str());
            return true;
        } catch (...) {
            LOG_ERROR("Failed to save config %s", name.c_str());
            return false;
        }
    }

    std::wstring GetBasePath() const { return base_path_; }

private:
    ConfigSystem() : base_path_(L"C:\\aimware\\") {}
    std::wstring base_path_;
};

} // namespace Aimware
