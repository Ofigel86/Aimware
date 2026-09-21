#include "netvars_manager.hpp"
#include <iostream>

namespace Aimware {

int NetvarManager::GetOffset(const char* tableName, const char* propName) {
    return GetProp(tableName, propName, nullptr);
}

int NetvarManager::GetProp(const char* tableName, const char* propName, RecvProp** outProp) {
    RecvTable* table = GetTable(tableName);
    if (!table) {
        LOG_WARN("Netvar table %s not found", tableName);
        return 0;
    }
    return GetProp(table, propName, outProp);
}

int NetvarManager::GetProp(RecvTable* recvTable, const char* propName, RecvProp** outProp) {
    if (!recvTable || !propName) return 0;
    return GetPropRecursive(recvTable, propName, outProp, 0);
}

int NetvarManager::GetPropRecursive(RecvTable* table, const char* propName, RecvProp** outProp, int accumulatedOffset) {
    if (!table) return 0;

    for (int i = 0; i < table->propCount; ++i) {
        RecvProp* prop = &table->props[i];
        if (!prop) continue;

        // If this prop is a datatable, recurse
        if (prop->dataTable && prop->dataTable->propCount > 0) {
            int result = GetPropRecursive(prop->dataTable, propName, outProp, accumulatedOffset + prop->offset);
            if (result != 0) {
                return result;
            }
        }

        // Direct match
        if (prop->name && strcmp(prop->name, propName) == 0) {
            if (outProp) *outProp = prop;
            return accumulatedOffset + prop->offset;
        }
    }
    return 0;
}

void* NetvarManager::GetClass(const char* className) {
    if (classes.empty() || !className) return nullptr;
    auto it = classes.find(className);
    if (it != classes.end()) return it->second;

    // Fallback linear search (case for unordered_map)
    for (auto& kv : classes) {
        if (kv.first == className) return kv.second;
    }
    return nullptr;
}

RecvTable* NetvarManager::GetTable(const char* tableName) {
    if (tables.empty() || !tableName) return nullptr;
    auto it = tables.find(tableName);
    if (it != tables.end()) return it->second;

    for (auto& kv : tables) {
        if (kv.first == tableName) return kv.second;
    }
    return nullptr;
}

bool NetvarManager::Initialize(IBaseClientDLL* client) {
    if (!client) {
        LOG_ERROR("NetvarManager::Initialize - null client");
        return false;
    }

    Clear();
    ClientClass* clientClass = client->GetAllClasses();
    if (!clientClass) {
        LOG_ERROR("GetAllClasses returned null");
        return false;
    }

    int count = 0;
    while (clientClass) {
        RecvTable* recvTable = clientClass->m_pRecvTable;
        if (recvTable && clientClass->m_pNetworkName) {
            classes.emplace(clientClass->m_pNetworkName, clientClass);
            tables.emplace(clientClass->m_pNetworkName, recvTable);
            ++count;
        }
        clientClass = clientClass->m_pNext;
    }

    LOG_SUCCESS("NetvarManager initialized: %d tables", count);
    return count > 0;
}

void NetvarManager::DumpNetvars(const char* filter) {
    LOG_INFO("=== Netvar Dump %s ===", filter ? filter : "ALL");
    for (auto& [name, table] : tables) {
        if (filter && name.find(filter) == std::string::npos) continue;
        LOG_INFO("Table: %s (%d props)", name.c_str(), table->propCount);
        for (int i = 0; i < table->propCount; ++i) {
            RecvProp* prop = &table->props[i];
            if (prop && prop->name) {
                LOG_INFO("  [%d] %s -> 0x%04X (type %d)", i, prop->name, prop->offset, prop->type);
            }
        }
    }
}

} // namespace Aimware
