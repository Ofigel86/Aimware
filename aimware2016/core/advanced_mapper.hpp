#pragma once
#define NOMINMAX
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <unordered_map>
#include "logger.hpp"
#include "memory_manager.hpp"
#include "pe_parser.hpp"
#include "decryptor.hpp"

namespace Aimware::Mapping {

// Advanced memory region with full info
struct AdvancedRegion {
    uintptr_t preferredBase;
    size_t size;
    const char* name;
    const uint8_t* sourceData;
    DWORD protection;
    bool isCode;
    bool isData;
    bool isStringSection;
    bool requiresDecryption;
    int decryptKey;

    // Runtime info
    void* actualBase = nullptr;
    bool isMapped = false;
    DWORD oldProtection = 0;
    PE::AimwareDumpAnalyzer::DumpInfo analysis;

    // Relocations
    std::vector<std::pair<uintptr_t, uint32_t>> relocations; // offset, type
};

// Manual mapper with relocation support
class AdvancedMapper {
public:
    static AdvancedMapper& Instance() {
        static AdvancedMapper instance;
        return instance;
    }

    // Add region to map
    void AddRegion(uintptr_t base, size_t size, const char* name, const uint8_t* data,
                   DWORD protect = PAGE_EXECUTE_READWRITE, bool isCode = false) {
        AdvancedRegion region;
        region.preferredBase = base;
        region.size = size;
        region.name = name;
        region.sourceData = data;
        region.protection = protect;
        region.isCode = isCode;
        region.isData = !isCode;
        region.isStringSection = (strstr(name, "String") != nullptr);
        region.requiresDecryption = false;
        region.decryptKey = 0;
        regions_.push_back(region);
        LOG_INFO("Mapper: Added region %s at 0x%08X size 0x%X %s",
            name, base, size, isCode ? "(CODE)" : "(DATA)");
    }

    // Try to decrypt region if needed
    bool DecryptRegion(AdvancedRegion& region) {
        if (!region.requiresDecryption || !region.sourceData) return true;

        LOG_INFO("Decrypting region %s with key 0x%X", region.name, region.decryptKey);

        // Create mutable copy
        std::vector<uint8_t> decrypted(region.sourceData, region.sourceData + region.size);

        auto encType = Crypto::Decryptor::DetectEncryption(decrypted.data(), decrypted.size());
        LOG_INFO("Detected encryption: %d for %s", (int)encType, region.name);

        switch (encType) {
            case Crypto::Decryptor::EncryptionType::SINGLE_XOR:
                Crypto::Decryptor::XorSingle(decrypted.data(), decrypted.size(), (uint8_t)region.decryptKey);
                break;
            case Crypto::Decryptor::EncryptionType::MULTI_XOR:
                // Would need multi-byte key
                break;
            default:
                LOG_WARN("Unknown encryption for %s, trying single XOR with key 0x%X", region.name, region.decryptKey);
                Crypto::Decryptor::XorSingle(decrypted.data(), decrypted.size(), (uint8_t)region.decryptKey);
                break;
        }

        // Allocate temporary storage for decrypted data
        // For now, we store it in a vector and update sourceData pointer
        // In real implementation, we'd keep decrypted data
        decryptedStorage_[region.name] = std::move(decrypted);
        region.sourceData = decryptedStorage_[region.name].data();

        return true;
    }

    // Map all regions with advanced handling
    bool MapAll(bool useFixedAddress = true) {
        LOG_INFO("=== Advanced Mapper: Mapping %zu regions (fixed=%s) ===",
            regions_.size(), useFixedAddress ? "YES" : "NO");

        bool allSuccess = true;

        for (auto& region : regions_) {
            if (!MapRegion(region, useFixedAddress)) {
                LOG_ERROR("Failed to map region %s", region.name);
                allSuccess = false;
                // Try alternative mapping without fixed address for debugging
                if (useFixedAddress) {
                    LOG_WARN("Retrying %s without fixed address...", region.name);
                    if (MapRegion(region, false)) {
                        LOG_WARN("Mapped %s at alternative address 0x%p (original 0x%08X)",
                            region.name, region.actualBase, region.preferredBase);
                        // For Aimware, fixed address is REQUIRED, so this is still failure
                        // But we keep it for analysis
                        UnmapRegion(region);
                        allSuccess = false;
                    }
                }
            }
        }

        if (allSuccess) {
            LOG_SUCCESS("All regions mapped successfully");
            ApplyRelocations();
            ApplyProtections();
            VerifyMapping();
        } else {
            LOG_ERROR("Some regions failed to map");
        }

        return allSuccess;
    }

    bool MapRegion(AdvancedRegion& region, bool useFixed) {
        // Analyze first
        if (region.sourceData) {
            region.analysis = PE::AimwareDumpAnalyzer::Analyze(
                region.preferredBase, region.size, region.name, region.sourceData);
            PE::AimwareDumpAnalyzer::PrintAnalysis(region.analysis);
        }

        // Decrypt if needed
        if (region.requiresDecryption) {
            if (!DecryptRegion(region)) {
                LOG_ERROR("Decryption failed for %s", region.name);
                return false;
            }
        }

        // Check if already mapped
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((LPCVOID)region.preferredBase, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT) {
                LOG_WARN("Region 0x%08X already committed (protect 0x%X), unmapping first",
                    region.preferredBase, mbi.Protect);
                // Try to unmap using NtUnmapViewOfSection for more thorough cleanup
                HMODULE ntdll = GetModuleHandleA("ntdll.dll");
                if (ntdll) {
                    auto NtUnmapViewOfSection = (NTSTATUS(WINAPI*)(HANDLE, PVOID))
                        GetProcAddress(ntdll, "NtUnmapViewOfSection");
                    if (NtUnmapViewOfSection) {
                        NtUnmapViewOfSection(GetCurrentProcess(), (PVOID)region.preferredBase);
                    }
                }
                VirtualFree((LPVOID)region.preferredBase, 0, MEM_RELEASE);
                Sleep(10);
            }
        }

        void* base = useFixed ? (void*)region.preferredBase : nullptr;
        void* allocated = VirtualAlloc(base, region.size, MEM_COMMIT | MEM_RESERVE, region.protection);

        if (!allocated) {
            DWORD err = GetLastError();
            LOG_ERROR("VirtualAlloc failed for %s at 0x%08X (size 0x%X) err=%d",
                region.name, region.preferredBase, region.size, err);
            return false;
        }

        if (useFixed && allocated != (void*)region.preferredBase) {
            LOG_ERROR("Allocated at 0x%p but required 0x%08X for %s",
                allocated, region.preferredBase, region.name);
            VirtualFree(allocated, 0, MEM_RELEASE);
            return false;
        }

        // Copy data
        if (region.sourceData) {
            memcpy(allocated, region.sourceData, region.size);
            LOG_INFO("Copied 0x%X bytes to %s at 0x%p", region.size, region.name, allocated);
        } else {
            memset(allocated, 0, region.size);
        }

        region.actualBase = allocated;
        region.isMapped = true;
        region.oldProtection = region.protection;

        // If code section, flush instruction cache
        if (region.isCode) {
            FlushInstructionCache(GetCurrentProcess(), allocated, region.size);
        }

        return true;
    }

    void UnmapRegion(AdvancedRegion& region) {
        if (region.isMapped && region.actualBase) {
            VirtualFree(region.actualBase, 0, MEM_RELEASE);
            region.isMapped = false;
            region.actualBase = nullptr;
        }
    }

    void UnmapAll() {
        LOG_INFO("Unmapping all regions...");
        for (auto& region : regions_) {
            UnmapRegion(region);
        }
        decryptedStorage_.clear();
        LOG_INFO("All regions unmapped");
    }

    // Apply relocations if dump has them
    void ApplyRelocations() {
        LOG_INFO("Applying relocations...");

        // For Aimware dumps, relocations are not standard PE relocs
        // They are fixed addresses that need to be fixed up if base changes
        // But since we use fixed base, relocations are not needed
        // However, we can still log potential relocations

        for (auto& region : regions_) {
            if (!region.isMapped || !region.sourceData) continue;

            // Scan for pointers that point to other regions
            int relocCount = 0;
            for (size_t i = 0; i + 4 <= region.size; i += 1) {
                uint32_t val = *(uint32_t*)((uint8_t*)region.actualBase + i);
                for (auto& other : regions_) {
                    if (val >= other.preferredBase && val < other.preferredBase + other.size) {
                        relocCount++;
                        // If actualBase differs from preferredBase, fix it
                        if (other.actualBase != (void*)other.preferredBase) {
                            uint32_t newVal = val - other.preferredBase + (uintptr_t)other.actualBase;
                            *(uint32_t*)((uint8_t*)region.actualBase + i) = newVal;
                        }
                        break;
                    }
                }
            }
            if (relocCount > 0) {
                LOG_INFO("Region %s has %d cross-region pointers", region.name, relocCount);
            }
        }
    }

    void ApplyProtections() {
        LOG_INFO("Applying final protections...");

        for (auto& region : regions_) {
            if (!region.isMapped) continue;

            DWORD finalProtect = region.protection;
            if (region.isCode) {
                finalProtect = PAGE_EXECUTE_READ;
            } else if (region.isStringSection) {
                finalProtect = PAGE_READONLY;
            } else {
                finalProtect = PAGE_READWRITE;
            }

            DWORD old;
            if (VirtualProtect(region.actualBase, region.size, finalProtect, &old)) {
                region.oldProtection = old;
                LOG_INFO("Region %s protection: 0x%X -> 0x%X", region.name, old, finalProtect);
            }
        }
    }

    void VerifyMapping() {
        LOG_INFO("=== Verifying Mapping ===");

        for (auto& region : regions_) {
            if (!region.isMapped) {
                LOG_ERROR("Region %s not mapped!", region.name);
                continue;
            }

            MEMORY_BASIC_INFORMATION mbi;
            if (!VirtualQuery(region.actualBase, &mbi, sizeof(mbi))) {
                LOG_ERROR("VirtualQuery failed for %s", region.name);
                continue;
            }

            LOG_INFO("Region %s: Base=0x%p AllocBase=0x%p Size=0x%X State=0x%X Protect=0x%X Type=0x%X",
                region.name, mbi.BaseAddress, mbi.AllocationBase, mbi.RegionSize,
                mbi.State, mbi.Protect, mbi.Type);

            // Verify content
            if (region.sourceData && region.actualBase) {
                if (memcmp(region.actualBase, region.sourceData, std::min(region.size, (size_t)0x100)) != 0) {
                    LOG_WARN("Region %s content mismatch in first 0x100 bytes", region.name);
                }
            }
        }
    }

    // Get region by name
    AdvancedRegion* GetRegion(const char* name) {
        for (auto& r : regions_) {
            if (strcmp(r.name, name) == 0) return &r;
        }
        return nullptr;
    }

    // Get region by address
    AdvancedRegion* GetRegionByAddress(uintptr_t addr) {
        for (auto& r : regions_) {
            if (addr >= r.preferredBase && addr < r.preferredBase + r.size) return &r;
        }
        return nullptr;
    }

    std::vector<AdvancedRegion>& GetRegions() { return regions_; }

private:
    AdvancedMapper() = default;
    std::vector<AdvancedRegion> regions_;
    std::unordered_map<std::string, std::vector<uint8_t>> decryptedStorage_;
};

// Compatibility wrapper for old MemoryManager
class MapperCompat {
public:
    static bool AllocateFixed(uintptr_t address, size_t size, const char* name, const void* src) {
        auto& mapper = AdvancedMapper::Instance();
        mapper.AddRegion(address, size, name, (const uint8_t*)src,
            PAGE_EXECUTE_READWRITE, strstr(name, "Code") != nullptr);
        return mapper.MapAll(true);
    }
};

} // namespace Aimware::Mapping
