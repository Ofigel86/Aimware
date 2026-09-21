#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include "core/sdk.hpp"
#include "core/logger.hpp"

namespace Aimware {

using namespace SDK;

class NetvarManager {
public:
    std::unordered_map<std::string, void*> classes;
    std::unordered_map<std::string, RecvTable*> tables;

    // Returns offset or 0 if not found
    int GetOffset(const char* tableName, const char* propName);

    // Returns offset and optionally fills prop pointer
    int GetProp(const char* tableName, const char* propName, RecvProp** outProp = nullptr);

    // Recursive search
    int GetProp(RecvTable* recvTable, const char* propName, RecvProp** outProp = nullptr);

    // Get class pointer
    void* GetClass(const char* className);

    // Get table pointer
    RecvTable* GetTable(const char* tableName);

    // Dump all netvars for debugging
    void DumpNetvars(const char* filter = nullptr);

    // Initialize from client
    bool Initialize(IBaseClientDLL* client);

    void Clear() {
        classes.clear();
        tables.clear();
    }

private:
    int GetPropRecursive(RecvTable* table, const char* propName, RecvProp** outProp, int accumulatedOffset);
};

// Compatibility alias
using netvars = NetvarManager;

} // namespace Aimware
