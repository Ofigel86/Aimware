#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <memory>
#include <optional>

namespace fs = std::filesystem;

// RAII Handle wrapper
class HandleWrapper {
public:
    HandleWrapper(HANDLE h = nullptr) : handle_(h) {}
    ~HandleWrapper() { Close(); }

    HandleWrapper(const HandleWrapper&) = delete;
    HandleWrapper& operator=(const HandleWrapper&) = delete;

    HandleWrapper(HandleWrapper&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    HandleWrapper& operator=(HandleWrapper&& other) noexcept {
        if (this != &other) {
            Close();
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    void Close() {
        if (handle_ && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
            handle_ = nullptr;
        }
    }

    HANDLE Get() const { return handle_; }
    operator HANDLE() const { return handle_; }
    bool IsValid() const { return handle_ && handle_ != INVALID_HANDLE_VALUE; }

    HANDLE* operator&() { return &handle_; }

private:
    HANDLE handle_ = nullptr;
};

// Fixed regions required by Aimware binary
struct FixedRegion {
    uintptr_t address;
    size_t size;
    const char* name;
};

constexpr FixedRegion kFixedRegions[] = {
    { 0x43AF0000, 90112,  "Data Section (b43AF0000)" },
    { 0x34E10000, 192512, "Code Section (b34E10000)" },
    { 0x7C4A0000, 122880, "CRT Helper (b7C4A0000)" },
    { 0x76ED0000, 110592, "String Resources (b76ED0000)" },
};

bool AllocateFixedRegions(HANDLE process) {
    printf("[+] Allocating fixed memory regions...\n");

    for (const auto& region : kFixedRegions) {
        // Check if already allocated
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(process, (LPCVOID)region.address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT) {
                printf("[*] Region %s at 0x%08X already committed, freeing...\n", region.name, region.address);
                VirtualFreeEx(process, (LPVOID)region.address, 0, MEM_RELEASE);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        void* allocated = VirtualAllocEx(process, (void*)region.address, region.size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!allocated) {
            DWORD err = GetLastError();
            printf("[-] Failed to allocate %s at 0x%08X (err=%d)\n", region.name, region.address, err);
            
            // Try to get more info
            if (err == 487) { // ERROR_INVALID_ADDRESS
                printf("    -> Address already in use or invalid. Try closing CS:GO and restarting.\n");
            }
            return false;
        }

        if (allocated != (void*)region.address) {
            printf("[!] Warning: allocated %s at 0x%p instead of 0x%08X\n", region.name, allocated, region.address);
            VirtualFreeEx(process, allocated, 0, MEM_RELEASE);
            return false;
        }

        printf("[+] Allocated %s at 0x%08X (size 0x%X)\n", region.name, region.address, region.size);
    }

    return true;
}

bool InjectModule(HANDLE process, const std::wstring& modulePath) {
    printf("[+] Injecting module: %ls\n", modulePath.c_str());

    size_t pathSize = (modulePath.size() + 1) * sizeof(wchar_t);
    void* remoteMem = VirtualAllocEx(process, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMem) {
        printf("[-] VirtualAllocEx for module path failed: %d\n", GetLastError());
        return false;
    }

    // RAII cleanup for remote memory
    auto cleanupRemote = [&]() {
        VirtualFreeEx(process, remoteMem, 0, MEM_RELEASE);
    };

    if (!WriteProcessMemory(process, remoteMem, modulePath.data(), pathSize, nullptr)) {
        printf("[-] WriteProcessMemory failed: %d\n", GetLastError());
        cleanupRemote();
        return false;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) {
        printf("[-] GetModuleHandleA(kernel32) failed\n");
        cleanupRemote();
        return false;
    }

    auto loadLibraryW = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryW");
    if (!loadLibraryW) {
        printf("[-] GetProcAddress(LoadLibraryW) failed\n");
        cleanupRemote();
        return false;
    }

    HandleWrapper remoteThread(CreateRemoteThread(process, nullptr, 0, loadLibraryW, remoteMem, 0, nullptr));
    if (!remoteThread.IsValid()) {
        printf("[-] CreateRemoteThread failed: %d\n", GetLastError());
        cleanupRemote();
        return false;
    }

    printf("[*] Remote thread created, waiting...\n");
    DWORD waitResult = WaitForSingleObject(remoteThread.Get(), 10000); // 10 sec timeout
    if (waitResult != WAIT_OBJECT_0) {
        printf("[-] WaitForSingleObject failed or timeout: %d\n", waitResult);
        cleanupRemote();
        return false;
    }

    DWORD exitCode = 0;
    GetExitCodeThread(remoteThread.Get(), &exitCode);
    if (exitCode == 0) {
        printf("[-] LoadLibraryW failed in remote process (exit code 0)\n");
        cleanupRemote();
        return false;
    }

    printf("[+] Module injected successfully at 0x%08X\n", exitCode);
    cleanupRemote();
    return true;
}

bool LaunchGameAndInject(const std::wstring& exePath, const std::wstring& exeParams, const std::wstring& modulePath, bool inject) {
    printf("[*] Launching: %ls %ls\n", exePath.c_str(), exeParams.c_str());

    if (!fs::exists(exePath)) {
        std::wcout << L"[-] Executable not found: " << exePath << std::endl;
        return false;
    }

    if (inject && !fs::exists(modulePath)) {
        std::wcout << L"[-] Module not found: " << modulePath << std::endl;
        return false;
    }

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    // CreateProcessW requires mutable buffer for command line
    std::wstring cmdLine = exePath + L" " + exeParams;
    std::vector<wchar_t> cmdLineBuf(cmdLine.begin(), cmdLine.end());
    cmdLineBuf.push_back(L'\0');

    BOOL created = CreateProcessW(
        exePath.c_str(),
        cmdLineBuf.data(),
        nullptr, nullptr,
        FALSE,
        CREATE_SUSPENDED,
        nullptr, nullptr,
        &si, &pi
    );

    if (!created) {
        printf("[-] CreateProcessW failed: %d\n", GetLastError());
        return false;
    }

    HandleWrapper process(pi.hProcess);
    HandleWrapper thread(pi.hThread);

    printf("[+] Game launched, PID=%d\n", pi.dwProcessId);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (inject) {
        if (!AllocateFixedRegions(process.Get())) {
            printf("[-] Failed to allocate fixed regions, terminating game...\n");
            TerminateProcess(process.Get(), 1);
            return false;
        }

        // Resume main thread BEFORE injection so game loads modules
        // Actually for this cheat we need to allocate before resume, then inject after resume
        ResumeThread(thread.Get());
        printf("[*] Game thread resumed, waiting for modules...\n");
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (!InjectModule(process.Get(), modulePath)) {
            printf("[-] Injection failed\n");
            return false;
        }

        printf("[+] Success! Game is running with Aimware.\n");
        printf("[*] You can close this injector, game will continue.\n");
        // Don't close process handle - let game run
        process.Close(); // But we close our handle, game continues
        thread.Close();
        return true;
    } else {
        ResumeThread(thread.Get());
        printf("[+] Game launched without injection (test mode)\n");
        process.Close();
        thread.Close();
        return true;
    }
}

void PrintUsage() {
    printf("Aimware 2016 Injector - Improved Version\n");
    printf("Usage:\n");
    printf("  injector.exe [options]\n");
    printf("Options:\n");
    printf("  --exe <path>       Path to csgo.exe (default: ./csgo.exe)\n");
    printf("  --dll <path>       Path to aimware.dll (default: ./aimware.dll)\n");
    printf("  --params <params>  Launch params (default: -insecure -steam)\n");
    printf("  --no-inject        Only launch game, don't inject\n");
    printf("  --help             Show this help\n");
}

int wmain(int argc, wchar_t* argv[]) {
    std::wstring exePath = L"./csgo.exe";
    std::wstring dllPath = L"./aimware.dll";
    std::wstring params = L"-insecure -steam -novid";
    bool doInject = true;

    // Parse args
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
        }
    }

    printf("=== Aimware 2016 Injector v2.0 ===\n");
    printf("EXE: %ls\n", exePath.c_str());
    printf("DLL: %ls\n", dllPath.c_str());
    printf("Params: %ls\n", params.c_str());
    printf("Inject: %s\n", doInject ? "YES" : "NO");
    printf("===============================\n");

    // Check if game is already running
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        bool found = false;
        if (Process32FirstW(snapshot, &pe)) {
            do {
                if (wcscmp(pe.szExeFile, L"csgo.exe") == 0) {
                    printf("[!] csgo.exe already running (PID %d), attempting to inject into existing process...\n", pe.th32ProcessID);
                    found = true;
                    // Try to open and inject into existing process
                    HandleWrapper existingProc(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID));
                    if (existingProc.IsValid()) {
                        if (doInject) {
                            if (AllocateFixedRegions(existingProc.Get())) {
                                if (InjectModule(existingProc.Get(), dllPath)) {
                                    printf("[+] Injected into existing process!\n");
                                    CloseHandle(snapshot);
                                    return 0;
                                }
                            }
                        }
                    }
                    break;
                }
            } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);
        if (found) {
            printf("[-] Failed to inject into existing process, try closing CS:GO first\n");
            return 1;
        }
    }

    bool success = LaunchGameAndInject(exePath, params, dllPath, doInject);

    if (success) {
        printf("\n[+] Done! Press Enter to exit...\n");
        std::cin.get();
        return 0;
    } else {
        printf("\n[-] Failed! Press Enter to exit...\n");
        std::cin.get();
        return 1;
    }
}

int main() {
    // Entry for non-unicode build
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    int result = wmain(argc, argv);
    LocalFree(argv);
    return result;
}
