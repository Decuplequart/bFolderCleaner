#pragma once

#include <windows.h>

#include <string>
#include <vector>

#define APP_NAME    L"BMS Folder Cleaner"
#define APP_VERSION L"1.0.1"

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

void ShowTestWindow(
    const std::vector<std::wstring>& files,
    const std::vector<std::wstring>& roots);

std::wstring GetExeDirectory();

void SaveBeatorajaPath(
    const std::wstring& path);

std::wstring LoadBeatorajaPath();

std::wstring SelectFolder();

std::vector<std::wstring> LoadBmsRoots(
    const std::wstring& configPath);

void AddSearchResults(
    const std::wstring& query,
    std::vector<std::wstring>& files);

std::wstring Utf8ToWide(
    const std::string& utf8);

void SaveRoots(
    const std::vector<std::wstring>& roots);

std::vector<std::wstring> LoadRoots();

std::string WideToUtf8(
    const std::wstring& wide);

void ClearFileList();

void AddFileToList(
    const std::wstring& fullPath);