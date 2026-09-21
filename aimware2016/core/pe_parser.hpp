#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include "logger.hpp"

namespace Aimware::PE {

// Minimal PE structures for parsing
struct DosHeader {
    uint16_t e_magic;
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    int32_t e_lfanew;
};

struct NtHeaders {
    uint32_t Signature;
    // FileHeader
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
    // OptionalHeader (PE32)
    uint16_t Magic;
    uint8_t MajorLinkerVersion;
    uint8_t MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint32_t BaseOfData;
    uint32_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    // ... more fields
};

struct SectionHeader {
    char Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
};

class PEParser {
public:
    static bool IsValidPE(const uint8_t* data, size_t size) {
        if (size < sizeof(DosHeader)) return false;
        auto dos = (DosHeader*)data;
        if (dos->e_magic != 0x5A4D) return false; // MZ
        if (dos->e_lfanew <= 0 || dos->e_lfanew + 6 > (int)size) return false;
        uint32_t* ntSig = (uint32_t*)(data + dos->e_lfanew);
        return *ntSig == 0x00004550; // PE\0\0
    }

    static std::optional<uint32_t> GetEntryPoint(const uint8_t* data, size_t size) {
        if (!IsValidPE(data, size)) return std::nullopt;
        auto dos = (DosHeader*)data;
        uint8_t* nt = (uint8_t*)(data + dos->e_lfanew);
        // Optional header is after FileHeader (20 bytes) + Signature (4)
        uint8_t* opt = nt + 4 + 20;
        uint32_t ep = *(uint32_t*)(opt + 16); // AddressOfEntryPoint at offset 16 in optional
        return ep;
    }

    static std::vector<SectionHeader> GetSections(const uint8_t* data, size_t size) {
        std::vector<SectionHeader> sections;
        if (!IsValidPE(data, size)) return sections;

        auto dos = (DosHeader*)data;
        uint8_t* nt = (uint8_t*)(data + dos->e_lfanew);
        uint16_t numSections = *(uint16_t*)(nt + 4 + 2);
        uint16_t optSize = *(uint16_t*)(nt + 4 + 16);
        uint8_t* sectionStart = nt + 4 + 20 + optSize;

        for (int i = 0; i < numSections && (sectionStart + sizeof(SectionHeader) * (i+1) <= data + size); ++i) {
            SectionHeader* sec = (SectionHeader*)(sectionStart + i * sizeof(SectionHeader));
            sections.push_back(*sec);
        }
        return sections;
    }

    static void DumpInfo(const uint8_t* data, size_t size, const char* name = "PE") {
        LOG_INFO("=== PE Info: %s (size 0x%X) ===", name, size);
        if (!IsValidPE(data, size)) {
            LOG_WARN("%s is not a valid PE", name);
            // Try to find PE signature inside
            for (size_t i = 0; i < size - 2; ++i) {
                if (data[i] == 'M' && data[i+1] == 'Z') {
                    LOG_INFO("Found MZ at offset 0x%X", i);
                    if (i + 0x3C < size) {
                        int32_t lfanew = *(int32_t*)(data + i + 0x3C);
                        if (i + lfanew + 2 < size) {
                            if (data[i+lfanew] == 'P' && data[i+lfanew+1] == 'E') {
                                LOG_INFO("Found PE at offset 0x%X + 0x%X = 0x%X", i, lfanew, i+lfanew);
                            }
                        }
                    }
                }
            }
            return;
        }

        auto ep = GetEntryPoint(data, size);
        if (ep) LOG_INFO("EntryPoint: 0x%08X", ep.value());

        auto sections = GetSections(data, size);
        LOG_INFO("Sections: %zu", sections.size());
        for (auto& sec : sections) {
            char nameBuf[9] = {0};
            memcpy(nameBuf, sec.Name, 8);
            LOG_INFO("  %s VA=0x%08X VS=0x%X Raw=0x%X Size=0x%X Char=0x%08X",
                nameBuf, sec.VirtualAddress, sec.VirtualSize, sec.PointerToRawData, sec.SizeOfRawData, sec.Characteristics);
        }
    }
};

// Aimware-specific dump analysis
class AimwareDumpAnalyzer {
public:
    struct DumpInfo {
        uintptr_t baseAddress;
        size_t size;
        const char* name;
        const uint8_t* data;
        double entropy;
        bool hasPointers;
        int pointerCount;
        bool hasCode;
        bool hasStrings;
    };

    static double CalculateEntropy(const uint8_t* data, size_t size) {
        if (!data || size == 0) return 0;
        int freq[256] = {0};
        for (size_t i = 0; i < size; ++i) freq[data[i]]++;
        double entropy = 0;
        for (int i = 0; i < 256; ++i) {
            if (freq[i] == 0) continue;
            double p = (double)freq[i] / size;
            entropy -= p * log2(p);
        }
        return entropy;
    }

    static int CountPointersToRegions(const uint8_t* data, size_t size) {
        struct Region { uintptr_t base, end; };
        Region regions[] = {
            {0x34E10000, 0x34E10000+192512},
            {0x43AF0000, 0x43AF0000+90112},
            {0x76ED0000, 0x76ED0000+110592},
            {0x7C4A0000, 0x7C4A0000+122880}
        };

        int count = 0;
        for (size_t i = 0; i + 4 <= size; i += 1) {
            uint32_t val = *(uint32_t*)(data + i);
            for (auto& r : regions) {
                if (val >= r.base && val < r.end) {
                    count++;
                    break;
                }
            }
        }
        return count;
    }

    static int CountCodePrologues(const uint8_t* data, size_t size) {
        // Common x86 prologues
        const uint8_t* patterns[] = {
            (uint8_t*)"\x55\x8B\xEC", // push ebp; mov ebp, esp
            (uint8_t*)"\x53\x8B\xDC", // push ebx; mov ebx, esp
            (uint8_t*)"\x56\x8B\xF1", // push esi; mov esi, ecx (thiscall)
        };
        int count = 0;
        for (size_t i = 0; i + 3 <= size; ++i) {
            for (auto pat : patterns) {
                if (memcmp(data + i, pat, 3) == 0) {
                    count++;
                    break;
                }
            }
        }
        return count;
    }

    static DumpInfo Analyze(uintptr_t base, size_t size, const char* name, const uint8_t* data) {
        DumpInfo info;
        info.baseAddress = base;
        info.size = size;
        info.name = name;
        info.data = data;
        info.entropy = CalculateEntropy(data, size);
        info.pointerCount = CountPointersToRegions(data, size);
        info.hasPointers = info.pointerCount > 10;
        int prologues = CountCodePrologues(data, size);
        info.hasCode = prologues > 10;
        info.hasStrings = false; // Would check for strings
        return info;
    }

    static void PrintAnalysis(const DumpInfo& info) {
        LOG_INFO("=== Dump Analysis: %s ===", info.name);
        LOG_INFO("Base: 0x%08X Size: 0x%X (%.1f KB)", info.baseAddress, info.size, info.size / 1024.0);
        LOG_INFO("Entropy: %.3f %s", info.entropy,
            info.entropy > 7.5 ? "(encrypted/compressed)" :
            info.entropy > 6.5 ? "(code/data mix)" : "(data)");
        LOG_INFO("Pointers to known regions: %d %s", info.pointerCount, info.hasPointers ? "(data section)" : "");
        LOG_INFO("Has code: %s", info.hasCode ? "YES" : "NO");
        PEParser::DumpInfo(info.data, info.size, info.name);
    }
};

} // namespace Aimware::PE
