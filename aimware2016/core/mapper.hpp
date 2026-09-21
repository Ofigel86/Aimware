#pragma once
#define NOMINMAX
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include "logger.hpp"

namespace Aimware::Mapping {

// Fixed regions required by Aimware binary - from RE report
struct FixedRegion {
    uintptr_t address;
    size_t size;
    const char* name;
};

// Default Aimware regions
constexpr FixedRegion kAimwareRegions[] = {
    { 0x43AF0000, 90112,  "DATA (b43AF0000)" },
    { 0x34E10000, 192512, "CODE (b34E10000)" },
    { 0x7C4A0000, 122880, "CRT (b7C4A0000)" },
    { 0x76ED0000, 110592, "STRING (b76ED0000)" },
};

// Simple, robust mapper for fixed addresses
class Mapper {
public:
    static Mapper& Instance() {
        static Mapper instance;
        return instance;
    }

    // Add custom region
    void AddRegion(uintptr_t addr, size_t size, const char* name) {
        regions_.push_back({ addr, size, name });
    }

    // Add default Aimware regions
    void AddDefaultRegions() {
        regions_.clear();
        for (auto& r : kAimwareRegions) {
            regions_.push_back(r);
        }
    }

    // Map all added regions at fixed addresses
    bool MapAll(const uint8_t* dataRegions[] = nullptr) {
        LOG_INFO("Mapper: mapping %zu regions", regions_.size());
        
        for (size_t i = 0; i < regions_.size(); ++i) {
            const auto& region = regions_[i];
            const uint8_t* src = dataRegions ? dataRegions[i] : nullptr;
            
            if (!MapRegion(region, src)) {
                LOG_ERROR("Mapper: failed %s at 0x%08X", region.name, region.address);
                UnmapAll();
                return false;
            }
        }
        
        LOG_SUCCESS("Mapper: all %zu regions mapped", regions_.size());
        return true;
    }

    // Map single region
    bool MapRegion(const FixedRegion& region, const uint8_t* srcData = nullptr) {
        // Check if already committed
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((LPCVOID)region.address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT) {
                LOG_WARN("Mapper: 0x%08X already committed, freeing", region.address);
                VirtualFree((LPVOID)region.address, 0, MEM_RELEASE);
                Sleep(10);
            }
        }

        void* allocated = VirtualAlloc((LPVOID)region.address, region.size,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (!allocated) {
            DWORD err = GetLastError();
            LOG_ERROR("Mapper: VirtualAlloc failed %s at 0x%08X size 0x%X err=%d",
                region.name, region.address, region.size, err);
            if (err == 487) {
                LOG_ERROR("  -> Address in use, close CS:GO or run as admin");
            }
            return false;
        }

        if (allocated != (LPVOID)region.address) {
            LOG_ERROR("Mapper: allocated at 0x%p != required 0x%08X for %s",
                allocated, region.address, region.name);
            VirtualFree(allocated, 0, MEM_RELEASE);
            return false;
        }

        if (srcData) {
            memcpy(allocated, srcData, region.size);
        } else {
            memset(allocated, 0, region.size);
        }

        mapped_.push_back(region);
        LOG_INFO("Mapper: %s mapped at 0x%08X (0x%X bytes)", region.name, region.address, region.size);
        return true;
    }

    // Unmap all
    void UnmapAll() {
        for (auto& region : mapped_) {
            VirtualFree((LPVOID)region.address, 0, MEM_RELEASE);
            LOG_INFO("Mapper: %s unmapped from 0x%08X", region.name, region.address);
        }
        mapped_.clear();
    }

    // Verify all mapped
    bool Verify() {
        bool ok = true;
        for (auto& region : regions_) {
            MEMORY_BASIC_INFORMATION mbi;
            if (!VirtualQuery((LPCVOID)region.address, &mbi, sizeof(mbi))) {
                LOG_ERROR("Mapper: VirtualQuery failed for %s", region.name);
                ok = false;
                continue;
            }
            if (mbi.State != MEM_COMMIT) {
                LOG_ERROR("Mapper: %s not committed", region.name);
                ok = false;
            }
        }
        if (ok) LOG_SUCCESS("Mapper: verification OK (%zu regions)", regions_.size());
        return ok;
    }

    // Get mapped regions
    const std::vector<FixedRegion>& GetMapped() const { return mapped_; }
    const std::vector<FixedRegion>& GetRegions() const { return regions_; }

    // Clear
    void Clear() {
        UnmapAll();
        regions_.clear();
    }

private:
    Mapper() = default;
    std::vector<FixedRegion> regions_;
    std::vector<FixedRegion> mapped_;
};

// Remote mapper for injector (VirtualAllocEx)
class RemoteMapper {
public:
    explicit RemoteMapper(HANDLE process) : process_(process) {}

    void AddRegion(uintptr_t addr, size_t size, const char* name) {
        regions_.push_back({ addr, size, name });
    }

    void AddDefaultRegions() {
        regions_.clear();
        for (auto& r : kAimwareRegions) regions_.push_back(r);
    }

    bool MapAll() {
        printf("[Mapper] Mapping %zu regions in remote process\n", regions_.size());
        
        for (auto& region : regions_) {
            if (!MapRegion(region)) {
                printf("[Mapper] Failed %s at 0x%08X\n", region.name, region.address);
                UnmapAll();
                return false;
            }
        }
        
        printf("[Mapper] All %zu regions mapped\n", regions_.size());
        return true;
    }

    bool MapRegion(const FixedRegion& region) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(process_, (LPCVOID)region.address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT) {
                printf("[Mapper] 0x%08X already committed, freeing\n", region.address);
                VirtualFreeEx(process_, (LPVOID)region.address, 0, MEM_RELEASE);
                Sleep(10);
                // Try NtUnmap for thorough cleanup
                HMODULE ntdll = GetModuleHandleA("ntdll.dll");
                if (ntdll) {
                    auto NtUnmap = (NTSTATUS(WINAPI*)(HANDLE, PVOID))GetProcAddress(ntdll, "NtUnmapViewOfSection");
                    if (NtUnmap) NtUnmap(process_, (PVOID)region.address);
                }
                Sleep(10);
            }
        }

        void* allocated = VirtualAllocEx(process_, (LPVOID)region.address, region.size,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (!allocated) {
            DWORD err = GetLastError();
            printf("[Mapper] VirtualAllocEx failed %s at 0x%08X err=%d\n", region.name, region.address, err);
            return false;
        }

        if (allocated != (LPVOID)region.address) {
            printf("[Mapper] Allocated at 0x%p != 0x%08X for %s\n", allocated, region.address, region.name);
            VirtualFreeEx(process_, allocated, 0, MEM_RELEASE);
            return false;
        }

        mapped_.push_back(region);
        printf("[Mapper] %s mapped at 0x%08X (0x%X)\n", region.name, region.address, region.size);
        return true;
    }

    void UnmapAll() {
        for (auto& r : mapped_) {
            VirtualFreeEx(process_, (LPVOID)r.address, 0, MEM_RELEASE);
        }
        mapped_.clear();
    }

    bool Verify() {
        bool ok = true;
        for (auto& region : regions_) {
            MEMORY_BASIC_INFORMATION mbi;
            if (!VirtualQueryEx(process_, (LPCVOID)region.address, &mbi, sizeof(mbi))) {
                printf("[Mapper] Query failed for %s\n", region.name);
                ok = false;
                continue;
            }
            if (mbi.State != MEM_COMMIT) {
                printf("[Mapper] %s not committed\n", region.name);
                ok = false;
            }
        }
        if (ok) printf("[Mapper] Verification OK (%zu regions)\n", regions_.size());
        return ok;
    }

private:
    HANDLE process_;
    std::vector<FixedRegion> regions_;
    std::vector<FixedRegion> mapped_;
};

} // namespace Aimware::Mapping
