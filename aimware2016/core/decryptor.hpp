#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include "logger.hpp"

namespace Aimware::Crypto {

// Decryption methods that Aimware might use
class Decryptor {
public:
    // Single-byte XOR
    static void XorSingle(uint8_t* data, size_t size, uint8_t key) {
        for (size_t i = 0; i < size; ++i) data[i] ^= key;
    }

    static std::vector<uint8_t> XorSingle(const uint8_t* data, size_t size, uint8_t key) {
        std::vector<uint8_t> out(size);
        for (size_t i = 0; i < size; ++i) out[i] = data[i] ^ key;
        return out;
    }

    // Multi-byte XOR with rolling key
    static void XorMulti(uint8_t* data, size_t size, const uint8_t* key, size_t keyLen) {
        for (size_t i = 0; i < size; ++i) data[i] ^= key[i % keyLen];
    }

    // XOR with position (common obfuscation)
    static void XorWithPos(uint8_t* data, size_t size, uint8_t baseKey) {
        for (size_t i = 0; i < size; ++i) {
            data[i] ^= (baseKey + (i & 0xFF));
        }
    }

    // Profile key decryption (as used in Aimware for strings)
    // From RE: xor cl, [eax] where eax is profile_t xor_key
    static std::string DecryptStringWithProfileKey(const char* encrypted, int xorKey) {
        if (!encrypted) return "";
        if (xorKey == 0) return std::string(encrypted); // No encryption

        std::string result;
        // Original: xor cl, [eax] - single byte key
        uint8_t key = (uint8_t)(xorKey & 0xFF);
        for (int i = 0; encrypted[i] != '\0'; ++i) {
            char dec = encrypted[i] ^ key;
            if (dec == '\0') break;
            // Filter non-printable
            if (dec < 32 && dec != '\0') {
                // Try with position also
                dec = encrypted[i] ^ key ^ (i & 0xFF);
            }
            result += dec;
            if (result.size() > 260) break; // Safety
        }
        return result;
    }

    // Try to brute-force single-byte XOR for known plaintext
    static std::optional<uint8_t> BruteForceXor(const uint8_t* data, size_t size, const uint8_t* knownPlain, size_t knownLen) {
        for (int key = 0; key < 256; ++key) {
            bool match = true;
            for (size_t i = 0; i < knownLen && i < size; ++i) {
                if ((data[i] ^ key) != knownPlain[i]) {
                    match = false;
                    break;
                }
            }
            if (match) return (uint8_t)key;
        }
        return std::nullopt;
    }

    // RC4-like decryption (common in cheats)
    static void RC4(uint8_t* data, size_t size, const uint8_t* key, size_t keyLen) {
        uint8_t S[256];
        for (int i = 0; i < 256; ++i) S[i] = i;
        int j = 0;
        for (int i = 0; i < 256; ++i) {
            j = (j + S[i] + key[i % keyLen]) % 256;
            std::swap(S[i], S[j]);
        }
        int i = 0;
        j = 0;
        for (size_t k = 0; k < size; ++k) {
            i = (i + 1) % 256;
            j = (j + S[i]) % 256;
            std::swap(S[i], S[j]);
            uint8_t K = S[(S[i] + S[j]) % 256];
            data[k] ^= K;
        }
    }

    // Aimware-specific: decrypt b76ED0000 string section
    // According to RE, strings are XOR encrypted with profile key
    static void DecryptStringSection(uint8_t* data, size_t size, int profileKey) {
        if (profileKey == 0) {
            LOG_WARN("Profile key is 0, string section may not be encrypted or uses different method");
            return;
        }

        LOG_INFO("Decrypting string section with profile key 0x%08X (%d)", profileKey, profileKey);
        // Try single-byte XOR first
        XorSingle(data, size, (uint8_t)(profileKey & 0xFF));

        // Check if we got readable strings
        int readable = 0;
        for (size_t i = 0; i < size - 4; ++i) {
            if (isprint(data[i]) && isprint(data[i+1]) && isprint(data[i+2]) && isprint(data[i+3])) {
                readable++;
            }
        }
        LOG_INFO("After XOR: readable sequences: %d", readable);
    }

    // Detect encryption type by entropy and heuristics
    enum class EncryptionType {
        NONE,
        SINGLE_XOR,
        MULTI_XOR,
        RC4,
        UNKNOWN_HIGH_ENTROPY
    };

    static EncryptionType DetectEncryption(const uint8_t* data, size_t size) {
        // Simple heuristic based on entropy and byte frequency
        // This is simplified - real detection would be more complex
        int freq[256] = {0};
        for (size_t i = 0; i < size; ++i) freq[data[i]]++;

        // Check for uniform distribution (high entropy = encrypted)
        double entropy = 0;
        for (int i = 0; i < 256; ++i) {
            if (freq[i] == 0) continue;
            double p = (double)freq[i] / size;
            entropy -= p * log2(p);
        }

        if (entropy < 6.0) return EncryptionType::NONE;
        if (entropy > 7.8) return EncryptionType::UNKNOWN_HIGH_ENTROPY;

        // Try to detect single-byte XOR by checking for common patterns
        // If after XOR with some key we get many 0x00 or common x86 bytes, it's likely XOR
        for (int key = 0; key < 256; ++key) {
            int zeroCount = 0;
            int commonCount = 0;
            for (size_t i = 0; i < std::min(size, (size_t)1000); ++i) {
                uint8_t dec = data[i] ^ key;
                if (dec == 0) zeroCount++;
                if (dec == 0x55 || dec == 0x8B || dec == 0xEC || dec == 0x83) commonCount++;
            }
            if (zeroCount > 50 || commonCount > 100) {
                return EncryptionType::SINGLE_XOR;
            }
        }

        return EncryptionType::UNKNOWN_HIGH_ENTROPY;
    }
};

// String decryptor for Aimware's encrypted strings at 0x76ED0000
class StringDecryptor {
public:
    struct EncryptedString {
        uintptr_t address;
        std::string decrypted;
        std::string encryptedRaw;
        int xorKey;
    };

    static std::vector<EncryptedString> DecryptAllStrings(uint8_t* stringSection, size_t size, uintptr_t baseAddress, int profileKey) {
        std::vector<EncryptedString> results;

        // According to RE, strings are at 0x76ED9CE0 (glove model) and 0x76ED9EA0 (chams material)
        // They are decrypted via: mov cl, [addr]; xor cl, [profile_key]

        // Scan for potential encrypted strings - sequences that after XOR become printable
        for (size_t i = 0; i < size - 10; ) {
            // Try to decrypt from i with profile key
            std::string decrypted;
            size_t j = i;
            uint8_t key = (uint8_t)(profileKey & 0xFF);

            while (j < size && j < i + 100) { // Max 100 char string
                uint8_t enc = stringSection[j];
                uint8_t dec = enc ^ key;

                if (dec == 0) break; // Null terminator
                if (!isprint(dec) && dec != 0) {
                    // Try alternative: XOR with key + position
                    dec = enc ^ key ^ (j & 0xFF);
                    if (!isprint(dec)) break;
                }
                decrypted += (char)dec;
                j++;
            }

            if (decrypted.size() >= 4 && decrypted.size() < 100) {
                // Check if it looks like a valid string (path, material, etc)
                bool valid = true;
                for (char c : decrypted) {
                    if (!isprint(c) && c != 0) { valid = false; break; }
                }
                if (valid) {
                    EncryptedString es;
                    es.address = baseAddress + i;
                    es.decrypted = decrypted;
                    es.encryptedRaw = std::string((char*)stringSection + i, j - i);
                    es.xorKey = profileKey;
                    results.push_back(es);
                    i = j + 1;
                    continue;
                }
            }
            i++;
        }

        return results;
    }

    static void DumpStrings(const std::vector<EncryptedString>& strings) {
        LOG_INFO("=== Decrypted Strings (%zu) ===", strings.size());
        for (auto& s : strings) {
            LOG_INFO("0x%08X: %s", s.address, s.decrypted.c_str());
        }
    }
};

} // namespace Aimware::Crypto
