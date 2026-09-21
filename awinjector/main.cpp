#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

struct HandleWrapper {
    HandleWrapper(HANDLE h = nullptr) : h_(h) {}
    ~HandleWrapper() { if (h_ && h_ != INVALID_HANDLE_VALUE) CloseHandle(h_); }
    HandleWrapper(const HandleWrapper&) = delete;
    HandleWrapper& operator=(const HandleWrapper&) = delete;
    HandleWrapper(HandleWrapper&& o) noexcept : h_(o.h_) { o.h_ = nullptr; }
    HANDLE Get() const { return h_; }
    bool Valid() const { return h_ && h_ != INVALID_HANDLE_VALUE; }
    HANDLE* operator&() { return &h_; }
    void Close() { if (Valid()) { CloseHandle(h_); h_ = nullptr; } }
private:
    HANDLE h_ = nullptr;
};

struct FixedRegion {
    uintptr_t addr;
    size_t size;
    const char* name;
};

constexpr FixedRegion kRegions[] = {
    { 0x43AF0000, 90112,  "DATA" },
    { 0x34E10000, 192512, "CODE" },
    { 0x7C4A0000, 122880, "CRT" },
    { 0x76ED0000, 110592, "STRING" },
};

bool MapFixed(HANDLE proc) {
    printf("[Mapper] Mapping %zu fixed regions\n", std::size(kRegions));
    for (auto& r : kRegions) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(proc, (LPCVOID)r.addr, &mbi, sizeof(mbi)) && mbi.State == MEM_COMMIT) {
            printf("[Mapper] 0x%08X in use, freeing\n", r.addr);
            VirtualFreeEx(proc, (LPVOID)r.addr, 0, MEM_RELEASE);
            Sleep(10);
            HMODULE ntdll = GetModuleHandleA("ntdll.dll");
            if (ntdll) {
                auto NtUnmap = (NTSTATUS(WINAPI*)(HANDLE, PVOID))GetProcAddress(ntdll, "NtUnmapViewOfSection");
                if (NtUnmap) NtUnmap(proc, (PVOID)r.addr);
            }
            Sleep(10);
        }
        void* a = VirtualAllocEx(proc, (LPVOID)r.addr, r.size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!a) {
            printf("[Mapper] Failed %s at 0x%08X err=%d\n", r.name, r.addr, GetLastError());
            return false;
        }
        if (a != (LPVOID)r.addr) {
            printf("[Mapper] Wrong addr 0x%p != 0x%08X for %s\n", a, r.addr, r.name);
            VirtualFreeEx(proc, a, 0, MEM_RELEASE);
            return false;
        }
        printf("[Mapper] %s -> 0x%08X (0x%X)\n", r.name, r.addr, r.size);
    }
    printf("[Mapper] All mapped OK\n");
    return true;
}

bool Inject(HANDLE proc, const std::wstring& dll) {
    printf("[Inject] %ls\n", dll.c_str());
    size_t sz = (dll.size() + 1) * sizeof(wchar_t);
    void* mem = VirtualAllocEx(proc, nullptr, sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!mem) return false;
    if (!WriteProcessMemory(proc, mem, dll.data(), sz, nullptr)) {
        VirtualFreeEx(proc, mem, 0, MEM_RELEASE);
        return false;
    }
    auto loadLib = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryW");
    HandleWrapper th(CreateRemoteThread(proc, nullptr, 0, loadLib, mem, 0, nullptr));
    if (!th.Valid()) {
        VirtualFreeEx(proc, mem, 0, MEM_RELEASE);
        return false;
    }
    WaitForSingleObject(th.Get(), 10000);
    DWORD code = 0;
    GetExitCodeThread(th.Get(), &code);
    VirtualFreeEx(proc, mem, 0, MEM_RELEASE);
    if (!code) {
        printf("[Inject] LoadLibrary failed\n");
        return false;
    }
    printf("[Inject] OK at 0x%08X\n", code);
    return true;
}

bool Launch(const std::wstring& exe, const std::wstring& params, const std::wstring& dll, bool doInject) {
    printf("[Launch] %ls %ls\n", exe.c_str(), params.c_str());
    if (!fs::exists(exe)) { std::wcout << L"EXE not found: " << exe << L"\n"; return false; }
    if (doInject && !fs::exists(dll)) { std::wcout << L"DLL not found: " << dll << L"\n"; return false; }

    STARTUPINFOW si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    std::wstring cmd = exe + L" " + params;
    std::vector<wchar_t> buf(cmd.begin(), cmd.end()); buf.push_back(0);
    
    if (!CreateProcessW(exe.c_str(), buf.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
        printf("CreateProcess failed %d\n", GetLastError());
        return false;
    }
    HandleWrapper proc(pi.hProcess), th(pi.hThread);
    printf("[Launch] PID %d\n", pi.dwProcessId);
    Sleep(500);
    
    if (doInject) {
        if (!MapFixed(proc.Get())) {
            printf("MapFixed failed\n");
            TerminateProcess(proc.Get(), 1);
            return false;
        }
        ResumeThread(th.Get());
        Sleep(2000);
        if (!Inject(proc.Get(), dll)) return false;
        printf("[+] SUCCESS - Game with Aimware\n");
        proc.Close(); th.Close();
        return true;
    } else {
        ResumeThread(th.Get());
        proc.Close(); th.Close();
        return true;
    }
}

int wmain(int argc, wchar_t* argv[]) {
    std::wstring exe = L"./csgo.exe";
    std::wstring dll = L"./aimware.dll";
    std::wstring params = L"-insecure -steam -novid";
    bool doInject = true;

    for (int i = 1; i < argc; ++i) {
        std::wstring a = argv[i];
        if (a == L"--exe" && i + 1 < argc) exe = argv[++i];
        else if (a == L"--dll" && i + 1 < argc) dll = argv[++i];
        else if (a == L"--params" && i + 1 < argc) params = argv[++i];
        else if (a == L"--no-inject") doInject = false;
        else if (a == L"--help") {
            printf("Aimware Mapper Injector\n");
            printf("  --exe <path> --dll <path> --params <str> --no-inject\n");
            printf("Fixed regions:\n");
            for (auto& r : kRegions) printf("  0x%08X 0x%X %s\n", r.addr, r.size, r.name);
            return 0;
        }
    }

    printf("=== Aimware Mapper Injector ===\n");
    printf("EXE: %ls\nDLL: %ls\n", exe.c_str(), dll.c_str());
    for (auto& r : kRegions) printf("Region: 0x%08X %s\n", r.addr, r.name);

    // Check running
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe{}; pe.dwSize = sizeof(pe);
        if (Process32FirstW(snap, &pe)) {
            do {
                if (wcscmp(pe.szExeFile, L"csgo.exe") == 0) {
                    printf("[!] csgo.exe running PID %d, injecting\n", pe.th32ProcessID);
                    HandleWrapper proc(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID));
                    if (proc.Valid() && doInject) {
                        if (MapFixed(proc.Get()) && Inject(proc.Get(), dll)) {
                            printf("[+] Injected into existing\n");
                            CloseHandle(snap);
                            return 0;
                        }
                    }
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }

    bool ok = Launch(exe, params, dll, doInject);
    printf("%s\nPress Enter...\n", ok ? "[+] Done" : "[-] Failed");
    std::cin.get();
    return ok ? 0 : 1;
}

int main() {
    int argc; wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    int r = wmain(argc, argv);
    LocalFree(argv);
    return r;
}
