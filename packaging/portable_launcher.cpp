#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
constexpr char kPackageMagic[] = "EGGPKG01";

struct PackageFooter
{
    char magic[sizeof(kPackageMagic) - 1];
    std::uint64_t archiveSize;
};

void ShowError(const wchar_t *message)
{
    MessageBoxW(nullptr, message, L"Eggscellent Catch", MB_OK | MB_ICONERROR);
}

bool WriteArchive(HANDLE executable, HANDLE archive, std::uint64_t offset, std::uint64_t size)
{
    LARGE_INTEGER position{};
    position.QuadPart = static_cast<LONGLONG>(offset);
    if (!SetFilePointerEx(executable, position, nullptr, FILE_BEGIN))
        return false;

    char buffer[64 * 1024];
    while (size > 0)
    {
        const DWORD requested = static_cast<DWORD>(
            size < sizeof(buffer) ? size : sizeof(buffer));
        DWORD bytesRead = 0;
        DWORD bytesWritten = 0;
        if (!ReadFile(executable, buffer, requested, &bytesRead, nullptr) ||
            bytesRead == 0 ||
            !WriteFile(archive, buffer, bytesRead, &bytesWritten, nullptr) ||
            bytesWritten != bytesRead)
        {
            return false;
        }
        size -= bytesRead;
    }
    return true;
}

std::wstring Quote(const std::wstring &value)
{
    std::wstring result = L"\"";
    for (wchar_t character : value)
    {
        if (character == L'\"')
            result += L'\\';
        result += character;
    }
    result += L'\"';
    return result;
}
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t executablePath[MAX_PATH];
    const DWORD pathLength = GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    if (pathLength == 0 || pathLength >= MAX_PATH)
    {
        ShowError(L"Could not locate the portable game file.");
        return 1;
    }

    HANDLE executable = CreateFileW(
        executablePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (executable == INVALID_HANDLE_VALUE)
    {
        ShowError(L"Could not read the portable game file.");
        return 1;
    }

    LARGE_INTEGER executableSize{};
    PackageFooter footer{};
    LARGE_INTEGER footerPosition{};
    bool packageIsValid =
        GetFileSizeEx(executable, &executableSize) &&
        executableSize.QuadPart >= static_cast<LONGLONG>(sizeof(footer));
    if (packageIsValid)
    {
        DWORD footerBytesRead = 0;
        footerPosition.QuadPart = executableSize.QuadPart - sizeof(footer);
        packageIsValid =
            SetFilePointerEx(executable, footerPosition, nullptr, FILE_BEGIN) &&
            ReadFile(executable, &footer, sizeof(footer), &footerBytesRead, nullptr) &&
            footerBytesRead == sizeof(footer) &&
            memcmp(footer.magic, kPackageMagic, sizeof(footer.magic)) == 0 &&
            footer.archiveSize <=
                static_cast<std::uint64_t>(executableSize.QuadPart - sizeof(footer));
    }

    if (!packageIsValid)
    {
        CloseHandle(executable);
        ShowError(L"The portable game file is incomplete or damaged.");
        return 1;
    }

    wchar_t temporaryPath[MAX_PATH];
    wchar_t temporaryDirectory[MAX_PATH];
    if (GetTempPathW(MAX_PATH, temporaryPath) == 0 ||
        GetTempFileNameW(temporaryPath, L"Egg", 0, temporaryDirectory) == 0)
    {
        CloseHandle(executable);
        ShowError(L"Could not create a temporary folder to run the game.");
        return 1;
    }
    DeleteFileW(temporaryDirectory);
    if (!CreateDirectoryW(temporaryDirectory, nullptr))
    {
        CloseHandle(executable);
        ShowError(L"Could not create a temporary folder to run the game.");
        return 1;
    }

    const std::wstring directory(temporaryDirectory);
    const std::wstring archivePath = directory + L"\\game.zip";
    const std::wstring scriptPath = directory + L"\\run-game.ps1";
    const std::wstring applicationPath = directory + L"\\app";

    HANDLE archive = CreateFileW(
        archivePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, nullptr);
    const std::uint64_t archiveOffset =
        static_cast<std::uint64_t>(executableSize.QuadPart) -
        sizeof(footer) - footer.archiveSize;
    const bool archiveWritten =
        archive != INVALID_HANDLE_VALUE &&
        WriteArchive(executable, archive, archiveOffset, footer.archiveSize);
    if (archive != INVALID_HANDLE_VALUE)
        CloseHandle(archive);
    CloseHandle(executable);

    if (!archiveWritten)
    {
        DeleteFileW(archivePath.c_str());
        RemoveDirectoryW(temporaryDirectory);
        ShowError(L"Could not unpack the portable game file.");
        return 1;
    }

    static constexpr wchar_t script[] =
        L"$ErrorActionPreference = 'Stop'\r\n"
        L"try {\r\n"
        L"    Expand-Archive -LiteralPath $args[0] -DestinationPath $args[1] -Force\r\n"
        L"    $game = Join-Path $args[1] 'EggscellentCatch.exe'\r\n"
        L"    if (-not (Test-Path -LiteralPath $game)) { throw 'The game executable is missing.' }\r\n"
        L"    Start-Process -FilePath $game -WorkingDirectory $args[1] -WindowStyle Normal -Wait\r\n"
        L"} catch {\r\n"
        L"    Add-Type -AssemblyName System.Windows.Forms\r\n"
        L"    [System.Windows.Forms.MessageBox]::Show($_.Exception.Message, 'Eggscellent Catch', 'OK', 'Error') | Out-Null\r\n"
        L"    exit 1\r\n"
        L"} finally {\r\n"
        L"    Remove-Item -LiteralPath $args[1] -Recurse -Force -ErrorAction SilentlyContinue\r\n"
        L"}\r\n";
    HANDLE scriptFile = CreateFileW(
        scriptPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    const wchar_t byteOrderMark[] = {static_cast<wchar_t>(0xFEFF)};
    DWORD bomBytesWritten = 0;
    DWORD scriptBytesWritten = 0;
    const bool scriptWritten =
        scriptFile != INVALID_HANDLE_VALUE &&
        WriteFile(scriptFile, byteOrderMark, sizeof(byteOrderMark),
                  &bomBytesWritten, nullptr) &&
        bomBytesWritten == sizeof(byteOrderMark) &&
        WriteFile(scriptFile, script, sizeof(script) - sizeof(wchar_t),
                  &scriptBytesWritten, nullptr) &&
        scriptBytesWritten == sizeof(script) - sizeof(wchar_t);
    if (scriptFile != INVALID_HANDLE_VALUE)
        CloseHandle(scriptFile);

    if (!scriptWritten)
    {
        DeleteFileW(archivePath.c_str());
        DeleteFileW(scriptPath.c_str());
        RemoveDirectoryW(temporaryDirectory);
        ShowError(L"Could not prepare the portable game to run.");
        return 1;
    }

    wchar_t systemDirectory[MAX_PATH];
    const UINT systemDirectoryLength =
        GetSystemDirectoryW(systemDirectory, MAX_PATH);
    if (systemDirectoryLength == 0 || systemDirectoryLength >= MAX_PATH)
    {
        DeleteFileW(archivePath.c_str());
        DeleteFileW(scriptPath.c_str());
        RemoveDirectoryW(temporaryDirectory);
        ShowError(L"Could not locate Windows PowerShell.");
        return 1;
    }

    const std::wstring powershellPath =
        std::wstring(systemDirectory) + L"\\WindowsPowerShell\\v1.0\\powershell.exe";
    std::wstring commandLine =
        Quote(powershellPath) +
        L" -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -WindowStyle Hidden -File " +
        Quote(scriptPath) + L" " + Quote(archivePath) + L" " + Quote(applicationPath);

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');
    const BOOL processStarted = CreateProcessW(
        powershellPath.c_str(), mutableCommand.data(), nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW, nullptr, directory.c_str(), &startupInfo, &processInfo);
    if (!processStarted)
    {
        DeleteFileW(archivePath.c_str());
        DeleteFileW(scriptPath.c_str());
        RemoveDirectoryW(temporaryDirectory);
        ShowError(L"Windows PowerShell could not be started. This portable build requires Windows PowerShell.");
        return 1;
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD powershellExitCode = 0;
    GetExitCodeProcess(processInfo.hProcess, &powershellExitCode);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    DeleteFileW(archivePath.c_str());
    DeleteFileW(scriptPath.c_str());
    RemoveDirectoryW(applicationPath.c_str());
    RemoveDirectoryW(temporaryDirectory);
    if (powershellExitCode != 0)
    {
        ShowError(L"Windows PowerShell could not start the portable game.");
        return 1;
    }
    return 0;
}
