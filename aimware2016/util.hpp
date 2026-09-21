#pragma once
#define NOMINMAX
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <cctype>
#include "core/logger.hpp"

namespace Aimware::Utils {

struct InterfaceReg {
    using InstantiateInterfaceFn = void* (*)();
    InstantiateInterfaceFn m_CreateFn;
    const char* m_pName;
    InterfaceReg* m_pNext;
};

// Modern, safe pattern scanner
class PatternScanner {
public:
    static std::optional<uintptr_t> Find(const char* module_name, const std::string& pattern) {
        HMODULE mod = GetModuleHandleA(module_name);
        if (!mod) {
            LOG_ERROR("Module %s not found for pattern scan", module_name);
            return std::nullopt;
        }

        MODULEINFO modInfo;
        if (!GetModuleInformation(GetCurrentProcess(), mod, &modInfo, sizeof(modInfo))) {
            LOG_ERROR("GetModuleInformation failed for %s", module_name);
            return std::nullopt;
        }

        uintptr_t start = (uintptr_t)modInfo.lpBaseOfDll;
        uintptr_t end = start + modInfo.SizeOfImage;
        auto parsed = ParsePattern(pattern);
        if (parsed.empty()) return std::nullopt;

        for (uintptr_t cur = start; cur < end; ++cur) {
            bool found = true;
            for (size_t i = 0; i < parsed.size(); ++i) {
                if (parsed[i].has_value() && *(uint8_t*)(cur + i) != parsed[i].value()) {
                    found = false;
                    break;
                }
            }
            if (found) return cur;
        }
        return std::nullopt;
    }

    static uintptr_t FindOrZero(const char* module_name, const char* pattern_str) {
        auto result = Find(module_name, pattern_str);
        return result.value_or(0);
    }

private:
    static std::vector<std::optional<uint8_t>> ParsePattern(const std::string& pattern) {
        std::vector<std::optional<uint8_t>> bytes;
        std::istringstream iss(pattern);
        std::string token;
        while (iss >> token) {
            if (token == "?" || token == "??") {
                bytes.push_back(std::nullopt);
            } else {
                try {
                    uint8_t byte = (uint8_t)std::stoi(token, nullptr, 16);
                    bytes.push_back(byte);
                } catch (...) {
                    // Invalid token, skip
                }
            }
        }
        return bytes;
    }
};

// Safe interface getter
template<typename T>
T* GetInterface(const char* mod_name, const char* interface_name, bool exact = false) {
    HMODULE mod = GetModuleHandleA(mod_name);
    if (!mod) {
        LOG_ERROR("Module %s not found for interface %s", mod_name, interface_name);
        return nullptr;
    }

    auto createInterfaceFn = (uintptr_t)GetProcAddress(mod, "CreateInterface");
    if (!createInterfaceFn) {
        LOG_ERROR("CreateInterface not found in %s", mod_name);
        return nullptr;
    }

    // x86 pattern: jmp [thunk] -> try to find InterfaceReg list
    // Fallback: brute force scan
    try {
        unsigned int jump_start = (unsigned int)(createInterfaceFn)+4;
        unsigned int jump_target = jump_start + *(unsigned int*)(jump_start + 1) + 5;
        InterfaceReg* reg_list = **reinterpret_cast<InterfaceReg***>(jump_target + 6);

        size_t part_len = strlen(interface_name);
        for (InterfaceReg* cur = reg_list; cur; cur = cur->m_pNext) {
            if (!cur->m_pName) continue;
            if (exact) {
                if (strcmp(cur->m_pName, interface_name) == 0) {
                    T* iface = reinterpret_cast<T*>(cur->m_CreateFn());
                    if (iface) {
                        LOG_INFO("Found interface %s -> %s", interface_name, cur->m_pName);
                        return iface;
                    }
                }
            } else {
                if (strncmp(cur->m_pName, interface_name, part_len) == 0) {
                    // Check version number after name
                    const char* version_part = cur->m_pName + part_len;
                    if (version_part[0] != '\0' && std::isdigit(version_part[0])) {
                        T* iface = reinterpret_cast<T*>(cur->m_CreateFn());
                        if (iface) {
                            LOG_INFO("Found interface %s -> %s at 0x%p", interface_name, cur->m_pName, iface);
                            return iface;
                        }
                    }
                }
            }
        }
    } catch (...) {
        LOG_ERROR("Exception while getting interface %s from %s", interface_name, mod_name);
    }

    LOG_ERROR("Interface %s not found in %s", interface_name, mod_name);
    return nullptr;
}

// Helper to get module info safely
inline bool GetModuleInfo(const char* mod_name, MODULEINFO& out_info) {
    HMODULE mod = GetModuleHandleA(mod_name);
    if (!mod) return false;
    return GetModuleInformation(GetCurrentProcess(), mod, &out_info, sizeof(out_info)) != 0;
}

// Legacy compatibility wrapper - returns 0 on failure
inline uint64_t find_signature(const char* szModule, const char* szSignature) {
    return PatternScanner::FindOrZero(szModule, szSignature);
}

template<typename T>
inline T* get_interface(const char* mod_name, const char* interface_name, bool exact = false) {
    return GetInterface<T>(mod_name, interface_name, exact);
}

} // namespace Aimware::Utils

// Keep old macros for compatibility
#define INRANGE(x, a, b) (x >= a && x <= b)
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0))
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))
