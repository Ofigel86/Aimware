#pragma once
// Legacy util.h - kept for compatibility, but forwards to new util.hpp
// New code should use util.hpp instead

#include "util.hpp"
#include "core/sdk.hpp"
#include "core/logger.hpp"

// Legacy compatibility - keep old classes for binary dump code
// These are now in core/sdk.hpp but we keep aliases here

using IAppSystem = Aimware::SDK::IAppSystem;
using ICvar = Aimware::SDK::ICvar;
using ITraceFilter = Aimware::SDK::ITraceFilter;
using ClientClass = Aimware::SDK::ClientClass;
using IBaseClientDLL = Aimware::SDK::IBaseClientDLL;
using CCStrike15ItemSystem = Aimware::SDK::CCStrike15ItemSystem;
using RecvTable = Aimware::SDK::RecvTable;
using RecvProp = Aimware::SDK::RecvProp;

// Keep old function signatures for compatibility
// find_signature and get_interface are now in util.hpp namespace
// But we provide global wrappers

// Note: find_signature is already defined in util.hpp as inline function
// get_interface is also defined there

// Additional legacy helpers
class CTraceFilterSkipTwoEntities : public ITraceFilter {
public:
    bool ShouldHitEntity(void* pEntityHandle, int contentsMask) override {
        return !(pEntityHandle == pSkip1 || pEntityHandle == pSkip2);
    }
    TraceType_t GetTraceType() const override {
        return TRACE_EVERYTHING;
    }
    void* pSkip1;
    void* pSkip2;
};

// Legacy macros kept for compatibility but new code should use PatternScanner
#ifndef INRANGE
#define INRANGE(x, a, b) (x >= a && x <= b)
#endif
#ifndef GETBITS
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0))
#endif
#ifndef GETBYTE
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))
#endif
