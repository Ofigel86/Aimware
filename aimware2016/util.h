#pragma once

class IAppSystem {
public:
	virtual bool                            Connect(void* factory) = 0;                                     // 0
	virtual void                            Disconnect() = 0;                                                           // 1
	virtual void* QueryInterface(const char* pInterfaceName) = 0;                             // 2
	virtual int /*InitReturnVal_t*/         Init() = 0;                                                                 // 3
	virtual void                            Shutdown() = 0;                                                             // 4
	virtual const void* /*AppSystemInfo_t*/ GetDependencies() = 0;                                                      // 5
	virtual int /*AppSystemTier_t*/         GetTier() = 0;                                                              // 6
	virtual void                            Reconnect(void* factory, const char* pInterfaceName) = 0;       // 7
	virtual void func8();
};

class ConCommandBase;
class ConCommand;
class ConVar;

typedef int CVarDLLIdentifier_t;

class IConsoleDisplayFunc
{
public:
	virtual void ColorPrint(const uint8_t* clr, const char* pMessage) = 0;
	virtual void Print(const char* pMessage) = 0;
	virtual void DPrint(const char* pMessage) = 0;
};

class ICvar : public IAppSystem
{
public:
	virtual CVarDLLIdentifier_t        AllocateDLLIdentifier() = 0; // 9
	virtual void                       RegisterConCommand(ConCommandBase* pCommandBase) = 0; //10
	virtual void                       UnregisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void                       UnregisterConCommands(CVarDLLIdentifier_t id) = 0;
	virtual const char* GetCommandLineValue(const char* pVariableName) = 0;
	virtual ConCommandBase* FindCommandBase(const char* name) = 0;
	virtual const ConCommandBase* FindCommandBase(const char* name) const = 0;
	virtual ConVar* FindVar(const char* var_name) = 0; //16
	virtual const ConVar* FindVar(const char* var_name) const = 0;
	virtual ConCommand* FindCommand(const char* name) = 0;
	virtual const ConCommand* FindCommand(const char* name) const = 0;
	virtual void                       InstallGlobalChangeCallback(void* callback) = 0;
	virtual void                       RemoveGlobalChangeCallback(void* callback) = 0;
	virtual void                       CallGlobalChangeCallbacks(ConVar* var, const char* pOldString, float flOldValue) = 0;
	virtual void                       InstallConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void                       RemoveConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void                       ConsoleColorPrintf(const void* clr, const char* pFormat, ...) const = 0;
	virtual void                       ConsolePrintf(const char* pFormat, ...) const = 0;
	virtual void                       ConsoleDPrintf(const char* pFormat, ...) const = 0;
	virtual void                       RevertFlaggedConVars(int nFlag) = 0;
};

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,
	TRACE_ENTITIES_ONLY,
	TRACE_EVERYTHING_FILTER_PROPS,
};

class ITraceFilter
{
public:
	virtual bool            ShouldHitEntity(void* pEntity, int contentsMask) = 0;
	virtual TraceType_t     GetTraceType() const = 0;
};

//class CTraceFilter : public ITraceFilter
//{
//public:
//	bool ShouldHitEntity(IClientEntity* pEntityHandle, int contentsMask)
//	{
//		return !(pEntityHandle == pSkip);
//	}
//
//	TraceType_t GetTraceType() const
//	{
//		return TRACE_EVERYTHING;
//	}
//
//	void* pSkip;
//};

class CCStrike15ItemSystem
{
public:
	virtual void* GetItemSchemaInterface() = 0;
};

class CTraceFilterSkipTwoEntities : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == pSkip1 || pEntityHandle == pSkip2);
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip1;
	void* pSkip2;
};

class InterfaceReg
{
private:
	using InstantiateInterfaceFn = void* (*)();
public:
	InstantiateInterfaceFn m_CreateFn;
	const char* m_pName;
	InterfaceReg* m_pNext;
};

template<typename T>
static T* get_interface(const char* mod_name, const char* interface_name, bool exact = false) {
	T* iface = nullptr;
	InterfaceReg* register_list;
	int part_match_len = strlen(interface_name); //-V103

	DWORD interface_fn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(mod_name), "CreateInterface"));

	if (!interface_fn) {
		return nullptr;
	}

	unsigned int jump_start = (unsigned int)(interface_fn)+4;
	unsigned int jump_target = jump_start + *(unsigned int*)(jump_start + 1) + 5;

	register_list = **reinterpret_cast<InterfaceReg***>(jump_target + 6);

	for (InterfaceReg* cur = register_list; cur; cur = cur->m_pNext) {
		if (exact == true) {
			if (strcmp(cur->m_pName, interface_name) == 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
		else {
			if (!strncmp(cur->m_pName, interface_name, part_match_len) && std::atoi(cur->m_pName + part_match_len) > 0) //-V106
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
	}
	return iface;
}


class ClientClass;
class IClientNetworkable;
class IClientEntity;

typedef IClientNetworkable* (*CreateClientClassFn)(int entnum, int serialNum);
typedef IClientNetworkable* (*CreateEventFn)();

class RecvTable;

class ClientClass
{
public:

	CreateClientClassFn m_pCreateFn;
	CreateEventFn m_pCreateEventFn;
	char* m_pNetworkName;
	RecvTable* m_pRecvTable;
	ClientClass* m_pNext;
	int m_ClassID;
};

class IBaseClientDLL
{
public:
	virtual int              Connect(void* appSystemFactory, void* pGlobals) = 0;
	virtual int              Disconnect(void) = 0;
	virtual int              Init(void* appSystemFactory, void* pGlobals) = 0;
	virtual void             PostInit() = 0;
	virtual void             Shutdown(void) = 0;
	virtual void             LevelInitPreEntity(char const* pMapName) = 0;
	virtual void             LevelInitPostEntity() = 0;
	virtual void             LevelShutdown(void) = 0;
	virtual ClientClass* GetAllClasses(void) = 0;
};

#define INRANGE(x, a, b) (x >= a && x <= b)  //-V1003
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0)) //-V1003
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

uint64_t find_signature(const char* szModule, const char* szSignature)
{
	MODULEINFO modInfo;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(szModule), &modInfo, sizeof(MODULEINFO));

	uintptr_t startAddress = (DWORD)modInfo.lpBaseOfDll;
	uintptr_t endAddress = startAddress + modInfo.SizeOfImage;

	const char* pat = szSignature;
	uintptr_t firstMatch = 0;

	for (auto pCur = startAddress; pCur < endAddress; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GETBYTE(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;
			else
				pat += 2;
		}
		else
		{
			pat = szSignature;
			firstMatch = 0;
		}
	}

	return 0;
}