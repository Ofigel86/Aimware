#include <Windows.h>
#include <iostream>
#include <intrin.h>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <deque>
#include <unordered_map>
#include <Psapi.h>
#include "b7C4A0000.h"
#include "b76ED0000.h"
#include "b43AF0000.h"
#include "b34E10000.h"

struct AwRender
{
	void* vtable;
	bool DidCreateFont;
	char pad[3];
	int Width;
	int Height;
	DWORD MenuFont;
	DWORD ESPFont;
};

struct AwGlobals
{

};

struct AwSkinChangerData
{
	bool filled;
	char pad[3];
	void* skin_data;
	int weapon_count;
	char pad1[0xBC];
	void* sequence_prop;
	void* sequence_proxy;
};