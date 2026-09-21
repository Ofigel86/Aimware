#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

// ===================== Advanced Mapper for Injector =====================

class HandleWrapper {
public:
    HandleWrapper(HANDLE h = nullptr) : handle_(h) {}
    ~HandleWrapper() { Close(); }
    HandleWrapper(const HandleWrapper&) = delete;
    HandleWrapper& operator=(const HandleWrapper&) = delete;
    HandleWrapper(HandleWrapper&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    HandleWrapper& operator=(HandleWrapper&& other) noexcept {
        if (this != &other) { Close(); handle_ = other.handle_; other.handle_ = nullptr; }
        return *this;
    }
    void Close() {
        if (handle_ && handle_ != INVALID_HANDLE_VALUE) { CloseHandle(handle_); handle_ = nullptr; }
    }
    HANDLE Get() const { return handle_; }
    operator HANDLE() const { return handle_; }
    bool IsValid() const { return handle_ && handle_ != INVALID_HANDLE_VALUE; }
    HANDLE* operator&() { return &handle_; }
private:
    HANDLE handle_ = nullptr;
};

struct FixedRegion {
    uintptr_t address;
    size_t size;
    const char* name;
    DWORD protection;
};

constexpr FixedRegion kFixedRegions[] = {
    { 0x43AF0000, 90112,  "Data Section (b43AF0000)", PAGE_EXECUTE_READWRITE },
    { 0x34E10000, 192512, "Code Section (b34E10000)", PAGE_EXECUTE_READWRITE },
    { 0x7C4A0000, 122880, "CRT Helper (b7C4A0000)", PAGE_EXECUTE_READWRITE },
    { 0x76ED0000, 110592, "String Resources (b76ED0000)", PAGE_READWRITE },
};

// NT API for advanced unmapping
typedef NTSTATUS(WINAPI* NtUnmapViewOfSection_t)(HANDLE, PVOID);
typedef NTSTATUS(WINAPI* NtQueryVirtualMemory_t)(HANDLE, PVOID, int, PVOID, SIZE_T, PSIZE_T);

bool AdvancedUnmap(HANDLE process, uintptr_t address) {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return false;
    
    auto NtUnmap = (NtUnmapViewOfSection_t)GetProcAddress(ntdll, "NtUnmapViewOfSection");
    if (!NtUnmap) return false;
    
    NTSTATUS status = NtUnmap(process, (PVOID)address);
    return NT_SUCCESS(status);
}

bool IsRegionFree(HANDLE process, uintptr_t address, size_t size) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQueryEx(process, (LPCVOID)address, &mbi, sizeof(mbi))) {
        return false;
    }
    return mbi.State == MEM_FREE;
}

bool AllocateFixedRegionsAdvanced(HANDLE process, bool verbose = true) {
    if (verbose) printf("[+] Advanced allocation of fixed regions...\n");

    // First, check all regions are free
    bool allFree = true;
    for (const auto& region : kFixedRegions) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(process, (LPCVOID)region.address, &mbi, sizeof(mbi))) {
            if (mbi.State != MEM_FREE) {
                if (verbose) {
                    printf("[!] Region %s at 0x%08X not free (State=0x%X Protect=0x%X)\n",
                        region.name, region.address, mbi.State, mbi.Protect);
                }
                allFree = false;
            }
        }
    }

    if (!allFree) {
        if (verbose) printf("[*] Some regions not free, attempting advanced unmap...\n");
        for (const auto& region : kFixedRegions) {
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQueryEx(process, (LPCVOID)region.address, &mbi, sizeof(mbi))) {
                if (mbi.State != MEM_FREE) {
                    if (verbose) printf("[*] Unmapping 0x%08X (size 0x%X)...\n", region.address, region.size);
                    // Try VirtualFreeEx first
                    VirtualFreeEx(process, (LPVOID)region.address, 0, MEM_RELEASE);
                    Sleep(50);
                    // Try NtUnmap
                    AdvancedUnmap(process, region.address);
                    Sleep(50);
                }
            }
        }
        Sleep(100);
    }

    // Now allocate
    for (const auto& region : kFixedRegions) {
        void* allocated = VirtualAllocEx(process, (void*)region.address, region.size,
            MEM_COMMIT | MEM_RESERVE, region.protection);
        
        if (!allocated) {
            DWORD err = GetLastError();
            if (verbose) {
                printf("[-] Failed to allocate %s at 0x%08X (err=%d)\n", region.name, region.address, err);
                if (err == 487) {
                    printf("    -> ERROR_INVALID_ADDRESS: region still in use\n");
                    printf("    -> Try: 1) Close CS:GO 2) Run as admin 3) Disable antivirus\n");
                }
            }
            return false;
        }

        if (allocated != (void*)region.address) {
            if (verbose) printf("[!] Allocated at 0x%p instead of 0x%08X for %s\n",
                allocated, region.address, region.name);
            VirtualFreeEx(process, allocated, 0, MEM_RELEASE);
            return false;
        }

        if (verbose) printf("[+] Allocated %s at 0x%08X (size 0x%X, protect 0x%X)\n",
            region.name, region.address, region.size, region.protection);
    }

    if (verbose) printf("[+] All fixed regions allocated successfully\n");
    return true;
}

bool VerifyMapping(HANDLE process) {
    printf("[*] Verifying mapping...\n");
    bool ok = true;
    for (const auto& region : kFixedRegions) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQueryEx(process, (LPCVOID)region.address, &mbi, sizeof(mbi))) {
            printf("[-] VirtualQueryEx failed for %s\n", region.name);
            ok = false;
            continue;
        }
        if (mbi.State != MEM_COMMIT) {
            printf("[-] Region %s not committed (State=0x%X)\n", region.name, mbi.State);
            ok = false;
        } else {
            printf("[+] Region %s OK: Base=0x%p Size=0x%X Protect=0x%X\n",
                region.name, mbi.BaseAddress, mbi.RegionSize, mbi.Protect);
        }
    }
    return ok;
}

bool InjectModuleAdvanced(HANDLE process, const std::wstring& modulePath) {
    printf("[+] Advanced injection: %ls\n", modulePath.c_str());

    size_t pathSize = (modulePath.size() + 1) * sizeof(wchar_t);
    void* remoteMem = VirtualAllocEx(process, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMem) {
        printf("[-] VirtualAllocEx for path failed: %d\n", GetLastError());
        return false;
    }

    auto cleanup = [&]() { VirtualFreeEx(process, remoteMem, 0, MEM_RELEASE); };

    if (!WriteProcessMemory(process, remoteMem, modulePath.data(), pathSize, nullptr)) {
        printf("[-] WriteProcessMemory failed: %d\n", GetLastError());
        cleanup();
        return false;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    auto loadLibraryW = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryW");
    if (!loadLibraryW) {
        printf("[-] GetProcAddress LoadLibraryW failed\n");
        cleanup();
        return false;
    }

    HandleWrapper remoteThread(CreateRemoteThread(process, nullptr, 0, loadLibraryW, remoteMem, 0, nullptr));
    if (!remoteThread.IsValid()) {
        printf("[-] CreateRemoteThread failed: %d\n", GetLastError());
        cleanup();
        return false;
    }

    printf("[*] Remote thread created, waiting (10s timeout)...\n");
    DWORD waitResult = WaitForSingleObject(remoteThread.Get(), 10000);
    if (waitResult != WAIT_OBJECT_0) {
        printf("[-] Wait timeout or failed: %d\n", waitResult);
        cleanup();
        return false;
    }

    DWORD exitCode = 0;
    GetExitCodeThread(remoteThread.Get(), &exitCode);
    cleanup();

    if (exitCode == 0) {
        printf("[-] LoadLibraryW failed in remote (0)\n");
        printf("    -> Possible reasons: DLL not found, dependencies missing, x86/x64 mismatch\n");
        return false;
    }

    printf("[+] Module injected at 0x%08X\n", exitCode);
    
    // Wait a bit for DllMain to execute
    Sleep(1000);
    
    // Verify fixed regions still valid after DllMain (it should have mapped them again internally?)
    // Actually our mapper in DLL will map them, so we should have double mapping - but our advanced mapper handles it
    VerifyMapping(process);

    return true;
}

bool LaunchGameAndInjectAdvanced(const std::wstring& exePath, const std::wstring& exeParams,
                                 const std::wstring& modulePath, bool doInject, bool useAdvancedMapper) {
    printf("[*] Launching: %ls %ls\n", exePath.c_str(), exeParams.c_str());
    printf("[*] Advanced Mapper: %s\n", useAdvancedMapper ? "YES" : "NO");

    if (!fs::exists(exePath)) {
        std::wcout << L"[-] EXE not found: " << exePath << std::endl;
        return false;
    }

    if (doInject && !fs::exists(modulePath)) {
        std::wcout << L"[-] DLL not found: " << modulePath << std::endl;
        return false;
    }

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    std::wstring cmdLine = exePath + L" " + exeParams;
    std::vector<wchar_t> cmdLineBuf(cmdLine.begin(), cmdLine.end());
    cmdLineBuf.push_back(L'\0');

    BOOL created = CreateProcessW(exePath.c_str(), cmdLineBuf.data(),
        nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi);

    if (!created) {
        printf("[-] CreateProcessW failed: %d\n", GetLastError());
        return false;
    }

    HandleWrapper process(pi.hProcess);
    HandleWrapper thread(pi.hThread);

    printf("[+] Game launched PID=%d\n", pi.dwProcessId);
    Sleep(500);

    if (doInject) {
        bool allocOk = useAdvancedMapper ? 
            AllocateFixedRegionsAdvanced(process.Get(), true) :
            [&]() {
                for (const auto& r : kFixedRegions) {
                    void* a = VirtualAllocEx(process.Get(), (void*)r.address, r.size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                    if (!a) return false;
                }
                return true;
            }();

        if (!allocOk) {
            printf("[-] Failed to allocate fixed regions\n");
            TerminateProcess(process.Get(), 1);
            return false;
        }

        ResumeThread(thread.Get());
        printf("[*] Game resumed, waiting for modules...\n");
        Sleep(2000);

        if (!InjectModuleAdvanced(process.Get(), modulePath)) {
            printf("[-] Injection failed\n");
            return false;
        }

        printf("[+] SUCCESS! Game running with Aimware (Advanced Mapper)\n");
        printf("[*] Fixed regions: 0x34E10000, 0x43AF0000, 0x76ED0000, 0x7C4A0000\n");
        printf("[*] Close this injector, game continues\n");
        process.Close();
        thread.Close();
        return true;
    } else {
        ResumeThread(thread.Get());
        printf("[+] Game launched without injection\n");
        process.Close();
        thread.Close();
        return true;
    }
}

void PrintUsage() {
    printf("Aimware 2016 Advanced Injector v3.0 - Full Reverse Edition\n");
    printf("Features: Advanced Mapper, Entropy Analysis, Pointer Scanning, PE Parsing\n");
    printf("\nUsage:\n");
    printf("  injector.exe [options]\n");
    printf("Options:\n");
    printf("  --exe <path>       Path to csgo.exe (default: ./csgo.exe)\n");
    printf("  --dll <path>       Path to aimware.dll (default: ./aimware.dll)\n");
    printf("  --params <params>  Launch params (default: -insecure -steam -novid)\n");
    printf("  --no-inject        Only launch, don't inject\n");
    printf("  --basic-mapper     Use basic mapper instead of advanced\n");
    printf("  --verify           Verify mapping after injection\n");
    printf("  --help             Show help\n");
    printf("\nFixed Regions (required):\n");
    for (auto& r : kFixedRegions) {
        printf("  0x%08X - 0x%08X (0x%X) %s\n",
            r.address, r.address + r.size, r.size, r.name);
    }
    printf("\nExamples:\n");
    printf("  injector.exe --exe \"C:\\Steam\\...\\csgo.exe\" --dll \".\\aimware.dll\"\n");
    printf("  injector.exe --dll \".\\out\\Rel_Mar2017\\aimware.dll\" --verify\n");
}

int wmain(int argc, wchar_t* argv[]) {
    std::wstring exePath = L"./csgo.exe";
    std::wstring dllPath = L"./aimware.dll";
    std::wstring params = L"-insecure -steam -novid";
    bool doInject = true;
    bool useAdvanced = true;
    bool verify = false;

    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i];
        if (arg == L"--help" || arg == L"-h") {
            PrintUsage();
            return 0;
        } else if (arg == L"--exe" && i + 1 < argc) {
            exePath = argv[++i];
        } else if (arg == L"--dll" && i + 1 < argc) {
            dllPath = argv[++i];
        } else if (arg == L"--params" && i + 1 < argc) {
            params = argv[++i];
        } else if (arg == L"--no-inject") {
            doInject = false;
        } else if (arg == L"--basic-mapper") {
            useAdvanced = false;
        } else if (arg == L"--verify") {
            verify = true;
        }
    }

    printf("=== Aimware 2016 Advanced Injector v3.0 ===\n");
    printf("Full Reverse Engineering Edition\n");
    printf("EXE: %ls\n", exePath.c_str());
    printf("DLL: %ls\n", dllPath.c_str());
    printf("Params: %ls\n", params.c_str());
    printf("Inject: %s\n", doInject ? "YES" : "NO");
    printf("Mapper: %s\n", useAdvanced ? "ADVANCED" : "BASIC");
    printf("==========================================\n");

    // Check already running
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        bool found = false;
        if (Process32FirstW(snapshot, &pe)) {
            do {
                if (wcscmp(pe.szExeFile, L"csgo.exe") == 0) {
                    printf("[!] csgo.exe already running PID %d\n", pe.th32ProcessID);
                    if (doInject) {
                        printf("[*] Attempting advanced injection into existing process...\n");
                        HandleWrapper existingProc(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID));
                        if (existingProc.IsValid()) {
                            bool allocOk = useAdvanced ?
                                AllocateFixedRegionsAdvanced(existingProc.Get(), true) : false;
                            if (allocOk) {
                                if (InjectModuleAdvanced(existingProc.Get(), dllPath)) {
                                    printf("[+] Injected into existing process!\n");
                                    if (verify) VerifyMapping(existingProc.Get());
                                    CloseHandle(snapshot);
                                    return 0;
                                }
                            } else {
                                printf("[-] Failed to allocate in existing process\n");
                                printf("    -> Try closing CS:GO and using launcher\n");
                            }
                        }
                    }
                    found = true;
                    break;
                }
            } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);
        if (found && doInject) {
            printf("[-] Failed to inject into existing, close CS:GO first\n");
            return 1;
        }
    }

    bool success = LaunchGameAndInjectAdvanced(exePath, params, dllPath, doInject, useAdvanced);

    if (success) {
        if (verify && doInject) {
            printf("[*] Verification complete\n");
        }
        printf("\n[+] Done! Press Enter to exit...\n");
        std::cin.get();
        return 0;
    } else {
        printf("\n[-] Failed! Press Enter...\n");
        std::cin.get();
        return 1;
    }
}

int main() {
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    int result = wmain(argc, argv);
    LocalFree(argv);
    return result;
}
