#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <cwchar>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

extern "C" BOOL WINAPI SetDllDirectoryW(LPCWSTR lpPathName);

static wchar_t* skipLauncherName(wchar_t* commandLine)
{
    if (!commandLine || *commandLine == L'\0')
        return commandLine;

    bool inQuotes = false;
    while (*commandLine) {
        if (*commandLine == L'"')
            inQuotes = !inQuotes;
        else if ((*commandLine == L' ' || *commandLine == L'\t') && !inQuotes)
            break;

        ++commandLine;
    }

    while (*commandLine == L' ' || *commandLine == L'\t')
        ++commandLine;
    return commandLine;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    wchar_t launcherPath[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, launcherPath, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return 1;

    wchar_t* separator = wcsrchr(launcherPath, L'\\');
    if (!separator)
        return 1;
    *separator = L'\0';

    wchar_t internalDirectory[MAX_PATH];
    wcsncpy(internalDirectory, launcherPath, ARRAY_SIZE(internalDirectory) - 1);
    internalDirectory[ARRAY_SIZE(internalDirectory) - 1] = L'\0';
    wcsncat(internalDirectory, L"\\_internal",
            ARRAY_SIZE(internalDirectory) - wcslen(internalDirectory) - 1);

    wchar_t applicationPath[MAX_PATH];
    wcsncpy(applicationPath, internalDirectory, ARRAY_SIZE(applicationPath) - 1);
    applicationPath[ARRAY_SIZE(applicationPath) - 1] = L'\0';
    wcsncat(applicationPath, L"\\kiwix-desktop.exe",
             ARRAY_SIZE(applicationPath) - wcslen(applicationPath) - 1);

    SetDllDirectoryW(internalDirectory);

    wchar_t* arguments = skipLauncherName(GetCommandLineW());
    wchar_t commandLine[MAX_PATH + 32768] = L"";
    wcsncat(commandLine, L"\"", ARRAY_SIZE(commandLine) - 1);
    wcsncat(commandLine, applicationPath,
            ARRAY_SIZE(commandLine) - wcslen(commandLine) - 1);
    wcsncat(commandLine, L"\"", ARRAY_SIZE(commandLine) - wcslen(commandLine) - 1);

    if (*arguments) {
        wcsncat(commandLine, L" ",
                ARRAY_SIZE(commandLine) - wcslen(commandLine) - 1);
        wcsncat(commandLine, arguments,
                ARRAY_SIZE(commandLine) - wcslen(commandLine) - 1);
    }

    STARTUPINFOW startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo = {};

    if (!CreateProcessW(applicationPath, commandLine, nullptr, nullptr, FALSE,
                        0, nullptr, internalDirectory, &startupInfo, &processInfo)) {
        return 1;
    }

    CloseHandle(processInfo.hThread);
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    CloseHandle(processInfo.hProcess);
    return static_cast<int>(exitCode);
}
