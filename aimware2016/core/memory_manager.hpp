#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <Psapi.h>
#include "../core/logger.hpp"

namespace Aimware {

struct FixedRegion {
    uintptr_t address;
    size_t size;
    const char* name;
    void* data;
};

class MemoryManager {
public:
    static MemoryManager& Instance() {
        static MemoryManager instance;
        return instance;
    }

    bool AllocateFixed(uintptr_t address, size_t size, const char* name, const void* src_data = nullptr) {
        LOG_INFO("Allocating %s at 0x%08X (size: 0x%X)", name, address, size);

        // Check if already allocated
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((LPCVOID)address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT) {
                LOG_WARN("Region 0x%08X already committed, freeing first", address);
                VirtualFree((LPVOID)address, 0, MEM_RELEASE);
            }
        }

        void* allocated = VirtualAlloc((LPVOID)address, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!allocated) {
            // Try without fixed address as fallback for debugging
            DWORD err = GetLastError();
            LOG_ERROR("Failed to allocate %s at fixed 0x%08X (err=%d)", name, address, err);
            allocated = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!allocated) {
                LOG_ERROR("Fallback allocation also failed for %s", name);
                return false;
            }
            LOG_WARN("Allocated %s at alternative address 0x%p instead of 0x%08X", name, allocated, address);
            // For fixed address requirement, this is failure
            VirtualFree(allocated, 0, MEM_RELEASE);
            return false;
        }

        if (allocated != (void*)address) {
            LOG_WARN("Allocated at 0x%p but requested 0x%08X", allocated, address);
        }

        if (src_data) {
            memcpy(allocated, src_data, size);
        } else {
            memset(allocated, 0, size);
        }

        regions_.push_back({ address, size, name, allocated });
        LOG_SUCCESS("%s allocated successfully", name);
        return true;
    }

    void* AllocateDynamic(size_t size, const char* tag = "dynamic") {
        void* mem = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!mem) {
            LOG_ERROR("Failed to allocate dynamic %s (size 0x%X)", tag, size);
            return nullptr;
        }
        memset(mem, 0, size);
        dynamic_regions_.push_back({ (uintptr_t)mem, size, tag, mem });
        return mem;
    }

    void* AllocateLagRecords() {
        constexpr size_t RECORD_SIZE = 0x3234;
        constexpr size_t MAX_PLAYERS = 64;
        constexpr size_t TOTAL_SIZE = RECORD_SIZE * MAX_PLAYERS;

        void* mem = AllocateDynamic(TOTAL_SIZE, "LagRecords");
        if (!mem) return nullptr;

        // Write pointer to fixed location expected by cheat
        *(PDWORD)0x43AF7A84 = (DWORD)mem;
        LOG_SUCCESS("LagRecords allocated at 0x%p (0x%X bytes)", mem, TOTAL_SIZE);
        return mem;
    }

    bool PatchBytes(uintptr_t address, const std::vector<uint8_t>& bytes) {
        DWORD old;
        if (!VirtualProtect((LPVOID)address, bytes.size(), PAGE_EXECUTE_READWRITE, &old)) {
            LOG_ERROR("VirtualProtect failed for patch at 0x%08X", address);
            return false;
        }
        memcpy((void*)address, bytes.data(), bytes.size());
        VirtualProtect((LPVOID)address, bytes.size(), old, &old);
        return true;
    }

    bool PatchByte(uintptr_t address, uint8_t byte) {
        return PatchBytes(address, { byte });
    }

    bool NopRange(uintptr_t start, uintptr_t end) {
        if (end <= start) return false;
        size_t size = end - start;
        std::vector<uint8_t> nops(size, 0x90);
        return PatchBytes(start, nops);
    }

    void FreeAll() {
        for (auto& region : regions_) {
            VirtualFree((LPVOID)region.address, 0, MEM_RELEASE);
        }
        for (auto& region : dynamic_regions_) {
            VirtualFree(region.data, 0, MEM_RELEASE);
        }
        regions_.clear();
        dynamic_regions_.clear();
        LOG_INFO("All memory regions freed");
    }

    ~MemoryManager() {
        // Don't auto-free on DLL unload to avoid crashes - explicit call needed
    }

private:
    MemoryManager() = default;
    std::vector<FixedRegion> regions_;
    std::vector<FixedRegion> dynamic_regions_;
};

} // namespace Aimware
