#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <stdlib.h>


DWORD GetProcId(const wchar_t* procName)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp(procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));

        }
    }
    CloseHandle(hSnap);
    return procId;
}
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}
int main() {
    SetConsoleTitleA("by exploreof");
    DWORD procId = GetProcId(L"cs2.exe");


    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"client.dll");

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
    
    uintptr_t dwLocalPlayerPawn = 0x17CF698; // https://github.com/a2x/cs2-dumper/blob/main/output/offsets.cs#L15
    uintptr_t m_lifeState = 0x328; // https://github.com/a2x/cs2-dumper/blob/main/output/client_dll.cs#L5652C45-L5652C50
    uintptr_t addressrate = moduleBase + dwLocalPlayerPawn; 


    while (true) {
        uintptr_t player;
        SIZE_T bytesRead;
        if (ReadProcessMemory(hProcess, (LPCVOID)addressrate, &player, sizeof(player), &bytesRead)) {
            int valueToWrite = 0;
            SIZE_T bytesWritten;
            if (WriteProcessMemory(hProcess, (LPVOID)(player + m_lifeState), &valueToWrite, sizeof(valueToWrite), &bytesWritten)) {
                std::cout << "[github.com/exploreof] success" << std::endl;
            }
            else {
                std::cout << "err" << std::endl;
            }
        }
        else {
            std::cerr << "error process memory" << std::endl;
        }
        Sleep(1000); 
    }

    
    CloseHandle(hProcess);
    return 0;
}