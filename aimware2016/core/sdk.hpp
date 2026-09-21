#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <Psapi.h>

namespace Aimware::SDK {

// ===================== Math =====================
struct Vector {
    float x, y, z;
    Vector() : x(0), y(0), z(0) {}
    Vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    float Length() const { return std::sqrt(x*x + y*y + z*z); }
    float Length2D() const { return std::sqrt(x*x + y*y); }
    float LengthSqr() const { return x*x + y*y + z*z; }
    float Dot(const Vector& v) const { return x*v.x + y*v.y + z*v.z; }
    Vector Cross(const Vector& v) const { return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }

    Vector operator+(const Vector& v) const { return Vector(x+v.x, y+v.y, z+v.z); }
    Vector operator-(const Vector& v) const { return Vector(x-v.x, y-v.y, z-v.z); }
    Vector operator*(float f) const { return Vector(x*f, y*f, z*f); }
    Vector operator/(float f) const { return Vector(x/f, y/f, z/f); }
    Vector& operator+=(const Vector& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    bool IsZero() const { return x==0 && y==0 && z==0; }
};

struct QAngle {
    float pitch, yaw, roll;
    QAngle() : pitch(0), yaw(0), roll(0) {}
    QAngle(float p, float y, float r) : pitch(p), yaw(y), roll(r) {}

    QAngle operator+(const QAngle& v) const { return QAngle(pitch+v.pitch, yaw+v.yaw, roll+v.roll); }
    QAngle operator-(const QAngle& v) const { return QAngle(pitch-v.pitch, yaw-v.yaw, roll-v.roll); }
    void Clamp() {
        if (pitch > 89.f) pitch = 89.f;
        if (pitch < -89.f) pitch = -89.f;
        while (yaw > 180.f) yaw -= 360.f;
        while (yaw < -180.f) yaw += 360.f;
        roll = 0;
    }
};

struct Matrix3x4 {
    float m[3][4];
    Vector GetOrigin() const { return Vector(m[0][3], m[1][3], m[2][3]); }
};

struct VMatrix {
    float m[4][4];
};

// ===================== CUserCmd =====================
struct CUserCmd {
    void* vmt;
    int command_number;
    int tick_count;
    QAngle viewangles;
    Vector aimdirection;
    float forwardmove;
    float sidemove;
    float upmove;
    int buttons;
    uint8_t impulse;
    int weaponselect;
    int weaponsubtype;
    int random_seed;
    short mousedx;
    short mousedy;
    bool hasbeenpredicted;
    char pad[24];
};

// ===================== LagRecord =====================
struct LagRecord {
    Vector origin;
    Vector velocity;
    Vector mins;
    Vector maxs;
    char pad0[0x31D4];
    QAngle eye_angles; // 0x3200 pitch, 0x3204 yaw
    char pad1[4];
    float lower_body_yaw; // 0x320C
    float lby_update_time; // 0x3210
    float simulation_time;
    float old_simulation_time;
    int flags;
    Matrix3x4 bone_matrix[128];
    bool valid;
    char pad2[0x23];
};
static_assert(sizeof(LagRecord) == 0x3234, "LagRecord size mismatch");

// ===================== Profile =====================
struct ProfileContext {
    int xor_key;
    int pad;
    wchar_t config_path[260];
    int pad2[64];
};

// ===================== Render =====================
struct AwRender {
    void* vtable;
    bool DidCreateFont;
    char pad[3];
    int Width;
    int Height;
    DWORD MenuFont;
    DWORD ESPFont;
    // Extended
    void* surface;
    void* font_manager;
};

struct AwGlobals {
    char pad[0x1000];
};

struct AwSkinChangerData {
    bool filled;
    char pad[3];
    void* skin_data;
    int weapon_count;
    char pad1[0xBC];
    void* sequence_prop;
    void* sequence_proxy;
    char pad2[0x40];
};

// ===================== Interfaces =====================
class IAppSystem {
public:
    virtual bool Connect(void* factory) = 0;
    virtual void Disconnect() = 0;
    virtual void* QueryInterface(const char* pInterfaceName) = 0;
    virtual int Init() = 0;
    virtual void Shutdown() = 0;
    virtual const void* GetDependencies() = 0;
    virtual int GetTier() = 0;
    virtual void Reconnect(void* factory, const char* pInterfaceName) = 0;
};

class ICvar : public IAppSystem {
public:
    virtual int AllocateDLLIdentifier() = 0;
    virtual void RegisterConCommand(void* pCommandBase) = 0;
    virtual void UnregisterConCommand(void* pCommandBase) = 0;
    virtual void UnregisterConCommands(int id) = 0;
    virtual const char* GetCommandLineValue(const char* pVariableName) = 0;
    virtual void* FindCommandBase(const char* name) = 0;
    virtual const void* FindCommandBase(const char* name) const = 0;
    virtual void* FindVar(const char* var_name) = 0;
    virtual const void* FindVar(const char* var_name) const = 0;
    virtual void* FindCommand(const char* name) = 0;
    virtual const void* FindCommand(const char* name) const = 0;
};

enum TraceType_t {
    TRACE_EVERYTHING = 0,
    TRACE_WORLD_ONLY,
    TRACE_ENTITIES_ONLY,
    TRACE_EVERYTHING_FILTER_PROPS,
};

class ITraceFilter {
public:
    virtual bool ShouldHitEntity(void* pEntity, int contentsMask) = 0;
    virtual TraceType_t GetTraceType() const = 0;
};

class ClientClass;
class RecvTable;

typedef void* (*CreateClientClassFn)(int entnum, int serialNum);
typedef void* (*CreateEventFn)();

class RecvTable;
class RecvProp;

class ClientClass {
public:
    CreateClientClassFn m_pCreateFn;
    CreateEventFn m_pCreateEventFn;
    char* m_pNetworkName;
    RecvTable* m_pRecvTable;
    ClientClass* m_pNext;
    int m_ClassID;
};

class IBaseClientDLL {
public:
    virtual int Connect(void* appSystemFactory, void* pGlobals) = 0;
    virtual int Disconnect(void) = 0;
    virtual int Init(void* appSystemFactory, void* pGlobals) = 0;
    virtual void PostInit() = 0;
    virtual void Shutdown(void) = 0;
    virtual void LevelInitPreEntity(char const* pMapName) = 0;
    virtual void LevelInitPostEntity() = 0;
    virtual void LevelShutdown(void) = 0;
    virtual ClientClass* GetAllClasses(void) = 0;
};

class CCStrike15ItemSystem {
public:
    virtual void* GetItemSchemaInterface() = 0;
};

// ===================== Netvars =====================
enum SendPropType {
    DPT_Int = 0,
    DPT_Float,
    DPT_Vector,
    DPT_VectorXY,
    DPT_String,
    DPT_Array,
    DPT_DataTable,
    DPT_Int64,
    DPT_NUMSendPropTypes
};

struct RecvProxyData {
    int pad;
    union {
        float _float;
        long _int;
        char* _string;
        void* data;
        float vec[3];
        int64_t int64;
    } value;
    int _pad2;
    SendPropType m_Type;
};

using RecvVarProxyFn = void(*)(RecvProxyData&, void*, void*);

struct RecvProp {
    char* name;
    int type;
    int flags;
    int stringBufferSize;
    int insideArray;
    const void* extraData;
    RecvProp* arrayProp;
    void* arrayLengthProxy;
    RecvVarProxyFn proxy;
    void* dataTableProxy;
    RecvTable* dataTable;
    int offset;
    int elementStride;
    int elementCount;
    const char* parentArrayPropName;
};

struct RecvTable {
    RecvProp* props;
    int propCount;
    void* decoder;
    char* netTableName;
    bool isInitialized;
    bool isInMainList;
};

} // namespace Aimware::SDK
