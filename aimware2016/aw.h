#pragma once
#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <Psapi.h>
#include <memory>

// Core - только маппер и логгер
#include "core/logger.hpp"
#include "core/mapper.hpp"
#include "core/sdk.hpp"
#include "core/config_system.hpp"

// Dumps
#include "b7C4A0000.h"
#include "b76ED0000.h"
#include "b43AF0000.h"
#include "b34E10000.h"

// Decompiled
#include "decompiled/aimware_decompiled.hpp"

// Managers
#include "hooking_manager.hpp"
#include "netvars_manager.hpp"
#include "util.hpp"

#define USE_DECOMPILED_ENGINE

using namespace Aimware;
using namespace Aimware::SDK;

struct AwRender {
    void* vtable;
    bool DidCreateFont;
    char pad[3];
    int Width;
    int Height;
    DWORD MenuFont;
    DWORD ESPFont;
};

struct AwGlobals {
    char _pad[0x1000];
};

struct AwSkinChangerData {
    bool filled;
    char pad[3];
    void* skin_data;
    int weapon_count;
    char pad1[0xBC];
    void* sequence_prop;
    void* sequence_proxy;
};

struct GlobalState {
    AwRender* render = nullptr;
    AwGlobals* global_ctx = nullptr;
    AwSkinChangerData* skinchanger_ctx = nullptr;

    void* engine_vgui = nullptr;
    std::unique_ptr<VMTHook> engine_vgui_hook;

    IBaseClientDLL* client = nullptr;
    std::unique_ptr<VMTHook> client_hook;

    void* client_mode = nullptr;
    std::unique_ptr<VMTHook> client_mode_hook;

    void* prediction = nullptr;
    std::unique_ptr<VMTHook> prediction_hook;

    void* surface = nullptr;
    std::unique_ptr<VMTHook> surface_hook;

    void* trace = nullptr;
    std::unique_ptr<VMTHook> trace_hook;

    void* studio_render = nullptr;
    std::unique_ptr<VMTHook> studio_render_hook;

    ICvar* cvars = nullptr;
    HWND window = nullptr;
    WNDPROC orig_wndproc = nullptr;

    NetvarManager netvars;
    std::vector<std::pair<uintptr_t, uintptr_t>> hooked_netvars;

    Mapping::Mapper* mapper = nullptr;

    bool initialized = false;
    bool panic = false;

    static GlobalState& Instance() {
        static GlobalState state;
        return state;
    }

    void Reset() {
        engine_vgui_hook.reset();
        client_hook.reset();
        client_mode_hook.reset();
        prediction_hook.reset();
        surface_hook.reset();
        trace_hook.reset();
        studio_render_hook.reset();
        hooked_netvars.clear();
        if (mapper) mapper->UnmapAll();
        initialized = false;
    }
};

namespace Aimware {
    bool InitializeMemoryDumps();
    bool InitializeInterfaces();
    bool InitializeNetvars();
    bool InitializeHooks();
    void Shutdown();
    void HookNetvar(const char* table, const char* var, uintptr_t original_addr, uintptr_t hook_fn);
    void UnhookNetvars();
    void FixImports();
    void FixAddresses();
    void FixConvars();
    void FixPostOEP();
}
