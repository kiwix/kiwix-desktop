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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    wchar_t outputPath[MAX_PATH];
    if (!GetEnvironmentVariableW(L"KIWIX_LAUNCHER_TEST_OUTPUT",
                                 outputPath, ARRAY_SIZE(outputPath)))
        return 2;

    HANDLE output = CreateFileW(outputPath, GENERIC_WRITE, 0, nullptr,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (output == INVALID_HANDLE_VALUE)
        return 3;

    wchar_t* commandLine = GetCommandLineW();
    DWORD bytesWritten = 0;
    WriteFile(output, commandLine, static_cast<DWORD>(wcslen(commandLine) * sizeof(wchar_t)),
              &bytesWritten, nullptr);
    CloseHandle(output);
    return 0;
}
