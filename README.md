
# LSASS Process Dumper and Encrypter

This repository contains two programs: one for dumping the `lsass.exe` process memory to a file and encrypting it using XOR encryption, and another for decrypting the encrypted dump file.

## Overview

### Programs

1. **sasDump**: Dumps the memory of the `lsass.exe` process and XOR encrypts the dump file with a key of `5` before saving it to disk.
2. **Decrypt**: Reads the encrypted dump file, decrypts it, and saves the decrypted content to a new file.

## Files

- `sasDum.cpp`: The program that dumps the `lsass.exe` process and encrypts the dump file.
- `decrypt.cpp`: The program that decrypts the encrypted dump file.

## Requirements

- Windows OS
- Visual Studio or any C++ compiler supporting Windows API and DbgHelp library

## Usage

### sasDump

1. **Compile `sasDum.cpp`**:
    ```sh
    cl /EHsc sasDum.cpp /link dbghelp.lib
    ```

2. **Run the executable**:
    ```sh
    sasDum.exe
    ```

3. **Output**: The encrypted dump file `encDump.txt` will be created in the same directory.

### Decrypt

1. **Compile `decrypt.cpp`**:
    ```sh
    cl /EHsc decrypt.cpp
    ```

2. **Run the executable**:
    ```sh
    decrypt.exe
    ```

3. **Output**: The decrypted dump file `encDump.txt` will be created in the same directory.

## Code Explanation

### sasDum.cpp

This program:
1. Finds the PID of the `lsass.exe` process.
2. Opens a handle to the `lsass.exe` process.
3. Uses `NtCreateProcessEx` to create a forked process of `lsass`.
4. Uses `MiniDumpWriteDump` to dump the memory of the forked process to a temporary file.
5. Reads the temporary dump file into memory.
6. XOR encrypts the dump file content with the key `5`.
7. Writes the encrypted content to `encDump.txt`.

### decrypt.cpp

This program:
1. Opens the encrypted dump file `encDump.txt`.
2. Reads the encrypted content into memory.
3. XOR decrypts the content with the key `5`.
4. Writes the decrypted content to `encDump.txt`.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## Contact

For any questions, please open an issue or contact the repository owner.
