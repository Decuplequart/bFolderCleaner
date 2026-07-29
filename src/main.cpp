#include <iostream>
#include <vector>

#include <windows.h>

#include <fcntl.h>
#include <io.h>

#include "functions.h"

int main()
{
    _setmode(
        _fileno(stdout),
        _O_U16TEXT);

    std::wstring configPath =
        GetExeDirectory() +
        L"\\config_sys.json";

    std::wstring savedPath =
        LoadBeatorajaPath();

    if (!savedPath.empty())
    {
        std::wstring savedConfig =
            savedPath +
            L"\\config_sys.json";

        if (GetFileAttributesW(
                savedConfig.c_str())
            != INVALID_FILE_ATTRIBUTES)
        {
            configPath =
                savedConfig;
        }
    }

    if (GetFileAttributesW(
            configPath.c_str())
        == INVALID_FILE_ATTRIBUTES)
    {
        MessageBoxW(
            nullptr,
            L"config_sys.json が見つかりません。\nbeatorajaフォルダを選択してください。",
            L"BMS Cleanup Tool",
            MB_OK |
            MB_ICONINFORMATION);

        std::wstring folder =
            SelectFolder();

        if (folder.empty())
        {
            return 0;
        }

        SaveBeatorajaPath(
            folder);

        configPath =
            folder +
            L"\\config_sys.json";

        if (GetFileAttributesW(
                configPath.c_str())
            == INVALID_FILE_ATTRIBUTES)
        {
            MessageBoxW(
                nullptr,
                configPath.c_str(),
                L"見つからなかったパス",
                MB_OK |
                MB_ICONERROR);

            return 1;
        }
    }

    std::vector<std::wstring> roots =
        LoadBmsRoots(
            configPath);

    if (roots.empty())
    {
        MessageBoxW(
            nullptr,
            L"bmsroot が取得できませんでした。",
            L"エラー",
            MB_OK |
            MB_ICONERROR);

        return 1;
    }

    std::vector<std::wstring> files;

    for (const auto& root : roots)
    {
        AddSearchResults(
            L"path:\"" +
            root +
            L"\" ext:ini",
            files);

        AddSearchResults(
            L"path:\"" +
            root +
            L"\" ext:db",
            files);
    }

    ShowTestWindow(
        files,
        roots);

    return 0;
}