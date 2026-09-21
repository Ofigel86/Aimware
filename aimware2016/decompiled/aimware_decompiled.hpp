#pragma once
#include "decompiled_sdk.hpp"
#include "aimbot_engine.hpp"
#include "antiaim_engine.hpp"
#include "resolver_engine.hpp"
#include "visuals_engine.hpp"
#include "skinchanger_engine.hpp"
#include "config_ui_engine.hpp"

namespace Aimware2016Decompiled {

inline void InitializeDecompiledEngine() {
    // Initialize all subsystems
    auto& aimbot = AimbotEngine::Instance();
    auto& antiaim = AntiAimEngine::Instance();
    auto& resolver = ResolverEngine::Instance();
    auto& visuals = VisualsEngine::Instance();
    auto& skinchanger = SkinChangerEngine::Instance();
    auto& config = ConfigUIEngine::Instance();

    // Set default configs
    aimbot.config.enabled = true;
    aimbot.config.fov = 30.f;
    aimbot.config.hitchance = 50.f;

    antiaim.config.enabled = false;
    antiaim.config.fakeLag = 6;

    visuals.config.enabled = true;
    visuals.config.box = true;
    visuals.config.healthBar = true;

    skinchanger.enabled = true;

    // Global context update
    GlobalContext::Instance().UpdateFromFixed();

    // Log success
    // Logger is in Aimware namespace, so use direct
    Aimware::Logger::Instance().Success("Decompiled engine initialized");
    Aimware::Logger::Instance().Info("  Aimbot: %s", aimbot.config.enabled ? "ON" : "OFF");
    Aimware::Logger::Instance().Info("  Visuals: %s", visuals.config.enabled ? "ON" : "OFF");
    Aimware::Logger::Instance().Info("  Resolver: %s", resolver.enabled ? "ON" : "OFF");
}

} // namespace Aimware2016Decompiled
