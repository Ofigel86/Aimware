#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include <filesystem>
#include <iostream>

bool load_module(HANDLE process, HANDLE thread, std::wstring module_name)
{
    void* aw1 = VirtualAllocEx(process, (void*)0x43AF0000, 90112, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!aw1)
    {
        printf("[-] Failed to allocate1\n");
        return 0;
    }
    void* aw2 = VirtualAllocEx(process, (void*)0x34E10000, 192512, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!aw2)
    {
        printf("[-] Failed to allocate2\n");
        return 0;
    }
    void* aw3 = VirtualAllocEx(process, (void*)0x7C4A0000, 122880, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!aw3)
    {
        printf("[-] Failed to allocate3\n");
        return 0;
    }
    void* aw4 = VirtualAllocEx(process, (void*)0x76ED0000, 110592, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!aw4)
    {
        printf("[-] Failed to allocate4\n");
        return 0;
    }

    ResumeThread(thread);

    void* module_name_alloc = VirtualAllocEx(process, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!module_name_alloc)
        return false;

    if (!WriteProcessMemory(process, module_name_alloc, module_name.data(), module_name.size() * sizeof(wchar_t), NULL))
        return false;

    if (CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryW, module_name_alloc, NULL, NULL) == INVALID_HANDLE_VALUE)
        return false;

    return true;
}

void launch(bool mp)
{
    std::wstring executable_name = L"./csgo.exe";
    std::wstring executable_parameters = L"";

    std::wstring module_name = L"./aimware.dll";

    if (!std::filesystem::exists(executable_name))
    {
        executable_name = L"./csgo.exe";
        if (!std::filesystem::exists(executable_name))
        {
            std::wcout << L"[-] executable not found." << std::endl;
            return;
        }
    }

    if (mp && !std::filesystem::exists(module_name))
    {
        std::wcout << L"[-] module not found." << std::endl;
        return;
    }

    STARTUPINFOW StartupInformation = {};
    StartupInformation.cb = sizeof(StartupInformation);

    PROCESS_INFORMATION ProcessInformation = {};

    if (CreateProcessW(executable_name.c_str(), (LPWSTR)L"-insecure -steam", NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &StartupInformation, &ProcessInformation))
    {
        std::wcout << L"[+] Launched game." << std::endl;

        Sleep(500);

        if (!mp || (mp && load_module(ProcessInformation.hProcess, ProcessInformation.hThread, module_name)))
        {
            std::wcout << L"[+] Success." << std::endl;
            return;
        }

        CloseHandle(ProcessInformation.hProcess);
    }
}
int main()
{
    launch(true);
    Sleep(20000);
    return 0;
}
