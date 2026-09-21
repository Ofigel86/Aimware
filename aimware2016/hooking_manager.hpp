#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>
#include <mutex>
#include "core/logger.hpp"

namespace Aimware {

// RAII guard for VirtualProtect
class ProtectGuard {
public:
    ProtectGuard(void* base, size_t len, DWORD protect) : base_(base), len_(len) {
        VirtualProtect(base, len, protect, &old_protect_);
    }
    ~ProtectGuard() {
        DWORD tmp;
        VirtualProtect(base_, len_, old_protect_, &tmp);
    }
    ProtectGuard(const ProtectGuard&) = delete;
    ProtectGuard& operator=(const ProtectGuard&) = delete;
private:
    void* base_;
    size_t len_;
    DWORD old_protect_;
};

// Modern VMT Hook Manager
class VMTHook {
public:
    VMTHook() : class_base_(nullptr), old_vt_(nullptr), new_vt_(nullptr), vt_size_(0) {}

    explicit VMTHook(void** class_base) : VMTHook() {
        Initialize(class_base);
    }

    explicit VMTHook(uintptr_t** class_base) : VMTHook() {
        Initialize(reinterpret_cast<void**>(class_base));
    }

    ~VMTHook() {
        Unhook();
        delete[] new_vt_;
    }

    VMTHook(const VMTHook&) = delete;
    VMTHook& operator=(const VMTHook&) = delete;

    VMTHook(VMTHook&& other) noexcept {
        MoveFrom(std::move(other));
    }

    VMTHook& operator=(VMTHook&& other) noexcept {
        if (this != &other) {
            Unhook();
            delete[] new_vt_;
            MoveFrom(std::move(other));
        }
        return *this;
    }

    bool Initialize(void** class_base) {
        if (!class_base || !*class_base) {
            LOG_ERROR("VMTHook::Initialize - null class base");
            return false;
        }

        class_base_ = class_base;
        old_vt_ = *reinterpret_cast<uintptr_t**>(class_base);
        vt_size_ = GetVTCount(old_vt_);

        if (vt_size_ == 0 || vt_size_ > 1024) {
            LOG_ERROR("VMTHook::Initialize - invalid VT size %zu", vt_size_);
            return false;
        }

        new_vt_ = new uintptr_t[vt_size_ + 1]();
        new_vt_[0] = old_vt_[-1]; // RTTI
        memcpy(&new_vt_[1], old_vt_, vt_size_ * sizeof(uintptr_t));

        try {
            ProtectGuard guard(class_base_, sizeof(uintptr_t), PAGE_READWRITE);
            *class_base_ = &new_vt_[1];
            LOG_INFO("VMTHook initialized: 0x%p with %zu methods", class_base_, vt_size_);
            return true;
        } catch (...) {
            LOG_ERROR("VMTHook::Initialize - exception during protect");
            delete[] new_vt_;
            new_vt_ = nullptr;
            return false;
        }
    }

    bool Initialize(uintptr_t** class_base) {
        return Initialize(reinterpret_cast<void**>(class_base));
    }

    void Unhook() {
        if (class_base_ && old_vt_) {
            try {
                ProtectGuard guard(class_base_, sizeof(uintptr_t), PAGE_READWRITE);
                *class_base_ = old_vt_;
                LOG_INFO("VMTHook unhooked: 0x%p", class_base_);
            } catch (...) {}
        }
    }

    void Rehook() {
        if (class_base_ && new_vt_) {
            try {
                ProtectGuard guard(class_base_, sizeof(uintptr_t), PAGE_READWRITE);
                *class_base_ = &new_vt_[1];
            } catch (...) {}
        }
    }

    void ClearClassBase() {
        class_base_ = nullptr;
    }

    size_t GetFuncCount() const { return vt_size_; }
    uintptr_t* GetOldVT() const { return old_vt_; }

    template<typename T>
    T GetFuncAddress(size_t index) const {
        if (index < vt_size_ && old_vt_) {
            return reinterpret_cast<T>(old_vt_[index]);
        }
        return nullptr;
    }

    uintptr_t GetFuncAddr(size_t index) const {
        if (index < vt_size_ && old_vt_) {
            return old_vt_[index];
        }
        return 0;
    }

    uintptr_t HookFunction(uintptr_t new_func, size_t index) {
        if (!new_vt_ || !old_vt_ || index >= vt_size_) {
            LOG_ERROR("HookFunction failed: invalid state or index %zu >= %zu", index, vt_size_);
            return 0;
        }
        new_vt_[index + 1] = new_func;
        LOG_INFO("Hooked index %zu: 0x%08X -> 0x%08X", index, old_vt_[index], new_func);
        return old_vt_[index];
    }

    template<typename T>
    uintptr_t HookFunction(T new_func, size_t index) {
        return HookFunction(reinterpret_cast<uintptr_t>(new_func), index);
    }

private:
    size_t GetVTCount(uintptr_t* vmt) {
        if (!vmt) return 0;
        size_t count = 0;
        // Safe upper bound and check for valid code pointers
        while (count < 1024) {
            uintptr_t func = vmt[count];
            if (!func) break;
            // Check if pointer is in executable memory
            MEMORY_BASIC_INFORMATION mbi;
            if (!VirtualQuery((LPCVOID)func, &mbi, sizeof(mbi))) break;
            if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
                // Might be end of VMT, but continue a bit to be safe
                // Check if it's obviously invalid
                if (func < 0x10000) break;
            }
            // Check for IS_INTRESOURCE-like invalid
            if (func > 0 && func < 0x1000) break;
            ++count;
        }
        return count;
    }

    void MoveFrom(VMTHook&& other) {
        class_base_ = other.class_base_;
        old_vt_ = other.old_vt_;
        new_vt_ = other.new_vt_;
        vt_size_ = other.vt_size_;
        other.class_base_ = nullptr;
        other.old_vt_ = nullptr;
        other.new_vt_ = nullptr;
        other.vt_size_ = 0;
    }

    void** class_base_;
    uintptr_t* old_vt_;
    uintptr_t* new_vt_;
    size_t vt_size_;
};

// Improved Shadow VMT Manager
class ShadowVTManager {
public:
    ShadowVTManager() : class_base_(nullptr), method_count_(0), shadow_vtable_(nullptr), original_vtable_(nullptr) {}
    explicit ShadowVTManager(void* base) : ShadowVTManager() { Setup(base); }

    ~ShadowVTManager() {
        RestoreTable();
        delete[] shadow_vtable_;
    }

    ShadowVTManager(const ShadowVTManager&) = delete;
    ShadowVTManager& operator=(const ShadowVTManager&) = delete;

    void Setup(void* base = nullptr) {
        if (base) class_base_ = base;
        if (!class_base_) return;

        original_vtable_ = *(uintptr_t**)class_base_;
        method_count_ = GetMethodCount(original_vtable_);

        if (method_count_ == 0 || method_count_ > 1024) {
            LOG_ERROR("ShadowVTManager: invalid method count %u", method_count_);
            return;
        }

        shadow_vtable_ = new uintptr_t[method_count_ + 1]();
        shadow_vtable_[0] = original_vtable_[-1];
        memcpy(&shadow_vtable_[1], original_vtable_, method_count_ * sizeof(uintptr_t));

        try {
            ProtectGuard guard(class_base_, sizeof(uintptr_t), PAGE_READWRITE);
            *(uintptr_t**)class_base_ = &shadow_vtable_[1];
            LOG_INFO("ShadowVTManager setup: %u methods", method_count_);
        } catch (...) {
            delete[] shadow_vtable_;
            shadow_vtable_ = nullptr;
            LOG_ERROR("ShadowVTManager setup failed");
        }
    }

    template<typename T>
    void Hook(uint32_t index, T method) {
        if (index < method_count_ && shadow_vtable_) {
            shadow_vtable_[index + 1] = reinterpret_cast<uintptr_t>(method);
        }
    }

    void Unhook(uint32_t index) {
        if (index < method_count_ && shadow_vtable_ && original_vtable_) {
            shadow_vtable_[index + 1] = original_vtable_[index];
        }
    }

    template<typename T>
    T GetOriginal(uint32_t index) {
        if (index < method_count_ && original_vtable_) {
            return reinterpret_cast<T>(original_vtable_[index]);
        }
        return nullptr;
    }

    void RestoreTable() {
        if (original_vtable_ && class_base_) {
            try {
                ProtectGuard guard(class_base_, sizeof(uintptr_t), PAGE_READWRITE);
                *(uintptr_t**)class_base_ = original_vtable_;
                original_vtable_ = nullptr;
            } catch (...) {}
        }
    }

private:
    uint32_t GetMethodCount(uintptr_t* vtable_start) {
        if (!vtable_start) return 0;
        uint32_t len = 0;
        while (len < 1024) {
            if (!vtable_start[len]) break;
            if (vtable_start[len] < 0x1000) break;
            MEMORY_BASIC_INFORMATION mbi;
            if (!VirtualQuery((LPCVOID)vtable_start[len], &mbi, sizeof(mbi))) break;
            ++len;
        }
        return len;
    }

    void* class_base_;
    uint32_t method_count_;
    uintptr_t* shadow_vtable_;
    uintptr_t* original_vtable_;
};

// Compatibility alias
using vmthook = VMTHook;

} // namespace Aimware
