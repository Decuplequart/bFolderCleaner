#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define ID_DELETE         1001
#define ID_CANCEL         1002
#define ID_ADDROOT        1003
#define ID_REMOVEROOT     1004
#define ID_RESCAN         1005
#define ID_SELECTALL      1006
#define ID_CHK_INI        1101
#define ID_CHK_DB         1102
#define ID_CHK_DSSTORE    1103
#define ID_CHK_DOTUNDER   1104
#define ID_CHK_THUMBS     1105
#define ID_CHK_DESKTOP    1106
#define ID_CHK_MACOSX     1107
#define ID_ROOTLIST       2001

#include "functions.h"

#include <fstream>
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>

#include "../include/Everything.h"

HWND g_listView = nullptr;
HWND g_rootList = nullptr;
HWND g_mainWindow = nullptr;
HWND g_selectAll = nullptr;

HWND g_statusText = nullptr;

HWND g_chkIni = nullptr;
HWND g_chkDb = nullptr;
HWND g_chkDsStore = nullptr;
HWND g_chkDotUnder = nullptr;
HWND g_chkThumbs = nullptr;
HWND g_chkDesktop = nullptr;
HWND g_chkMacosx = nullptr;

std::vector<std::wstring> g_files;
std::vector<std::wstring> g_roots;

void ClearFileList();

void AddFileToList(
    const std::wstring& fullPath);

void UpdateWindowTitle();

void UpdateSelectedCount();

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        switch (id)
        {
        case ID_DELETE:
        {
            std::vector<std::wstring>
                selectedFiles;

            int itemCount =
                ListView_GetItemCount(
                    g_listView);

            for (int i = 0;
                i < itemCount;
                i++)
            {
                if (ListView_GetCheckState(
                        g_listView,
                        i))
                {
                    selectedFiles.push_back(
                        g_files[i]);
                }
            }

            if (selectedFiles.empty())
            {
                MessageBoxW(
                    hwnd,
                    L"削除対象が選択されていません。",
                    L"確認",
                    MB_OK |
                    MB_ICONINFORMATION);

                return 0;
            }

            std::wstring message =
                L"選択された " +
                std::to_wstring(
                    selectedFiles.size()) +
                L" 件を削除します。\n\n"
                L"この操作は元に戻せません。";

            int answer =
                MessageBoxW(
                    hwnd,
                    message.c_str(),
                    L"削除確認",
                    MB_YESNO |
                    MB_ICONQUESTION);

            if (answer != IDYES)
            {
                return 0;
            }

            int deletedCount = 0;

            for (const auto& file :
                selectedFiles)
            {
                try
                {
                    if (std::filesystem::remove_all(
                            file) > 0)
                    {
                        deletedCount++;
                    }
                }
                catch (...)
                {
                }
            }

            SendMessageW(
                hwnd,
                WM_COMMAND,
                ID_RESCAN,
                0);

            std::wstring result =
                L"削除完了: " +
                std::to_wstring(
                    deletedCount) +
                L" 件";

            MessageBoxW(
                hwnd,
                result.c_str(),
                L"完了",
                MB_OK |
                MB_ICONINFORMATION);

            return 0;
        }

        case ID_CANCEL:
        {
            DestroyWindow(hwnd);
            return 0;
        }

        case ID_ADDROOT:
        {
            std::wstring folder =
                SelectFolder();

            if (!folder.empty())
            {
                g_roots.push_back(
                    folder);

                SaveRoots(
                    g_roots);

                SendMessageW(
                    g_rootList,
                    LB_ADDSTRING,
                    0,
                    (LPARAM)folder.c_str());

                SendMessageW(
                    hwnd,
                    WM_COMMAND,
                    ID_RESCAN,
                    0);
            }

            return 0;
        }

        case ID_REMOVEROOT:
        {
            int index =
                (int)SendMessageW(
                    g_rootList,
                    LB_GETCURSEL,
                    0,
                    0);

            if (index != LB_ERR)
            {
                g_roots.erase(
                    g_roots.begin() +
                    index);

                SaveRoots(
                    g_roots);

                SendMessageW(
                    g_rootList,
                    LB_DELETESTRING,
                    index,
                    0);
                
                SendMessageW(
                    hwnd,
                    WM_COMMAND,
                    ID_RESCAN,
                    0);
            }

            return 0;
        }

        case ID_RESCAN:
        {
            ClearFileList();

            for (const auto& root : g_roots)
            {
            if (SendMessageW(
                    g_chkIni,
                    BM_GETCHECK,
                    0,
                    0) == BST_CHECKED)
            {
                AddSearchResults(
                    root + L" *.ini",
                    g_files);
            }

            }

            for (const auto& file : g_files)
            {
                AddFileToList(
                    file);
            }

            UpdateWindowTitle();
            UpdateSelectedCount();

            return 0;
        }

        case ID_SELECTALL:
        {
            BOOL checked =
                (BOOL)SendMessageW(
                    g_selectAll,
                    BM_GETCHECK,
                    0,
                    0);

            int count =
                ListView_GetItemCount(
                    g_listView);

            for (int i = 0;
                i < count;
                i++)
            {
                ListView_SetCheckState(
                    g_listView,
                    i,
                    checked == BST_CHECKED);
            }
            
            UpdateSelectedCount();

            return 0;
        }

        case ID_ROOTLIST:
            return 0;
        }

        break;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    }

    return DefWindowProcW(
        hwnd,
        msg,
        wParam,
        lParam);
}

void ClearFileList()
{
    ListView_DeleteAllItems(
        g_listView);

    g_files.clear();
}

void AddFileToList(
    const std::wstring& fullPath)
{
    std::wstring fileName =
        fullPath;

    size_t pos =
        fileName.find_last_of(
            L"\\/");

    if (pos != std::wstring::npos)
    {
        fileName =
            fileName.substr(
                pos + 1);
    }

    LVITEMW item = {};

    item.mask = LVIF_TEXT;
    item.iItem =
        ListView_GetItemCount(
            g_listView);

    item.pszText =
        const_cast<LPWSTR>(
            fileName.c_str());

    int row =
        ListView_InsertItem(
            g_listView,
            &item);

    ListView_SetItemText(
        g_listView,
        row,
        1,
        const_cast<LPWSTR>(
            fullPath.c_str()));

    ListView_SetCheckState(
        g_listView,
        row,
        TRUE);
}

void UpdateWindowTitle()
{
    std::wstring title =
        L"ini/DB Killer (" +
        std::to_wstring(
            g_files.size()) +
        L"件)";

    SetWindowTextW(
        g_mainWindow,
        title.c_str());
}

void UpdateSelectedCount()
{
    int checked = 0;

    int count =
        ListView_GetItemCount(
            g_listView);

    for (int i = 0;
        i < count;
        i++)
    {
        if (ListView_GetCheckState(
                g_listView,
                i))
        {
            checked++;
        }
    }

    std::wstring text =
        L"選択: " +
        std::to_wstring(
            checked) +
        L"件";

    SetWindowTextW(
        g_statusText,
        text.c_str());
}

void ShowTestWindow(
    const std::vector<std::wstring>& files,
    const std::vector<std::wstring>& roots)
{

    g_files = files;

    g_roots = LoadRoots();

    if (g_roots.empty())
    {
        g_roots = roots;

        SaveRoots(
            g_roots);
    }

    INITCOMMONCONTROLSEX icc = {};

    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES;

    InitCommonControlsEx(&icc);

    const wchar_t CLASS_NAME[] =
        L"BMSCleanupWindow";

    WNDCLASSW wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);
    
    HWND hwnd =
        CreateWindowExW(
            0,
            CLASS_NAME,
            L"ini/DB Killer",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1200,
            700,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);
    
    g_mainWindow = hwnd;

    g_rootList =
        CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"LISTBOX",
            L"",
            WS_CHILD |
            WS_VISIBLE |
            LBS_NOTIFY |
            WS_VSCROLL,
            10,
            10,
            1160,
            100,
            hwnd,
            (HMENU)ID_ROOTLIST,
            GetModuleHandleW(nullptr),
            nullptr);
    
    for (const auto& root : g_roots)
    {
        SendMessageW(
            g_rootList,
            LB_ADDSTRING,
            0,
            (LPARAM)root.c_str());
    }
    
    g_statusText =
        CreateWindowW(
            L"STATIC",
            L"選択: 0件",
            WS_CHILD |
            WS_VISIBLE,
            10,
            620,
            250,
            25,
            hwnd,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

    CreateWindowW(
        L"BUTTON",
        L"追加",
        WS_CHILD |
        WS_VISIBLE |
        BS_PUSHBUTTON,
        10,
        110,
        100,
        25,
        hwnd,
        (HMENU)ID_ADDROOT,
        GetModuleHandleW(nullptr),
        nullptr);

    CreateWindowW(
        L"BUTTON",
        L"ルート削除",
        WS_CHILD |
        WS_VISIBLE |
        BS_PUSHBUTTON,
        120,
        110,
        120,
        25,
        hwnd,
        (HMENU)ID_REMOVEROOT,
        GetModuleHandleW(nullptr),
        nullptr);

    CreateWindowW(
        L"BUTTON",
        L"再検索",
        WS_CHILD |
        WS_VISIBLE |
        BS_PUSHBUTTON,
        250,
        110,
        100,
        25,
        hwnd,
        (HMENU)ID_RESCAN,
        GetModuleHandleW(nullptr),
        nullptr);

    g_selectAll =
        CreateWindowW(
            L"BUTTON",
            L"全選択",
            WS_CHILD |
            WS_VISIBLE |
            BS_AUTOCHECKBOX,
            370,
            112,
            120,
            25,
            hwnd,
            (HMENU)ID_SELECTALL,
            GetModuleHandleW(nullptr),
            nullptr);

    g_chkIni =
        CreateWindowW(
            L"BUTTON",
            L"ini",
            WS_CHILD |
            WS_VISIBLE |
            BS_AUTOCHECKBOX,
            500,
            112,
            60,
            25,
            hwnd,
            (HMENU)ID_CHK_INI,
            GetModuleHandleW(nullptr),
            nullptr);

    g_chkIni =
        CreateWindowW(
            L"BUTTON",
            L"ini",
            WS_CHILD |
            WS_VISIBLE |
            BS_AUTOCHECKBOX,
            500,
            112,
            60,
            25,
            hwnd,
            (HMENU)ID_CHK_INI,
            GetModuleHandleW(nullptr),
            nullptr);

    g_listView =
        CreateWindowExW(
            WS_EX_CLIENTEDGE,
            WC_LISTVIEWW,
            L"",
            WS_CHILD |
            WS_VISIBLE |
            LVS_REPORT,
            10,
            140,
            1160,
            470,
            hwnd,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

    ListView_SetExtendedListViewStyle(
        g_listView,
        LVS_EX_FULLROWSELECT |
        LVS_EX_GRIDLINES |
        LVS_EX_CHECKBOXES);

    LVCOLUMNW col = {};

    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.cx = 200;
    col.pszText = const_cast<LPWSTR>(L"ファイル名");

    ListView_InsertColumn(
        g_listView,
        0,
        &col);

    col.cx = 900;
    col.pszText = const_cast<LPWSTR>(L"フルパス");

    ListView_InsertColumn(
        g_listView,
        1,
        &col);

    for (const auto& file : files)
    {
        AddFileToList(file);
    }

    UpdateWindowTitle();
    UpdateSelectedCount();

    CreateWindowW(
        L"BUTTON",
        L"削除",
        WS_CHILD |
        WS_VISIBLE |
        BS_PUSHBUTTON,
        900,
        620,
        120,
        30,
        hwnd,
        (HMENU)ID_DELETE,
        GetModuleHandleW(nullptr),
        nullptr);

    CreateWindowW(
        L"BUTTON",
        L"キャンセル",
        WS_CHILD |
        WS_VISIBLE |
        BS_PUSHBUTTON,
        1040,
        620,
        120,
        30,
        hwnd,
        (HMENU)ID_CANCEL,
        GetModuleHandleW(nullptr),
        nullptr);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;

    while (GetMessageW(
        &msg,
        nullptr,
        0,
        0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

std::wstring Utf8ToWide(
    const std::string& utf8)
{
    if (utf8.empty())
    {
        return L"";
    }

    int size =
        MultiByteToWideChar(
            CP_UTF8,
            0,
            utf8.c_str(),
            -1,
            nullptr,
            0);

    std::wstring result(
        size - 1,
        L'\0');

    MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8.c_str(),
        -1,
        &result[0],
        size);

    return result;
}

std::string WideToUtf8(
    const std::wstring& wide)
{
    if (wide.empty())
    {
        return "";
    }

    int size =
        WideCharToMultiByte(
            CP_UTF8,
            0,
            wide.c_str(),
            -1,
            nullptr,
            0,
            nullptr,
            nullptr);

    std::string result(
        size - 1,
        '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wide.c_str(),
        -1,
        &result[0],
        size,
        nullptr,
        nullptr);

    return result;
}

std::wstring GetExeDirectory()
{
    wchar_t path[MAX_PATH];

    GetModuleFileNameW(
        nullptr,
        path,
        MAX_PATH);

    std::wstring exePath = path;

    size_t pos = exePath.find_last_of(L"\\/");

    return exePath.substr(0, pos);
}

void SaveRoots(
    const std::vector<std::wstring>& roots)
{
    std::wstring path =
        GetExeDirectory() +
        L"\\roots.txt";

    std::ofstream file(
        path.c_str(),
        std::ios::binary);

    if (!file)
    {
        return;
    }

    for (const auto& root : roots)
    {
        file
            << WideToUtf8(root)
            << "\n";
    }
}

std::vector<std::wstring> LoadRoots()
{
    std::vector<std::wstring> roots;

    std::wstring path =
        GetExeDirectory() +
        L"\\roots.txt";

    std::ifstream file(
        path.c_str(),
        std::ios::binary);

    if (!file)
    {
        return roots;
    }

    std::string line;

    while (std::getline(
        file,
        line))
    {
        if (!line.empty())
        {
            roots.push_back(
                Utf8ToWide(line));
        }
    }

    return roots;
}

void SaveBeatorajaPath(
    const std::wstring& path)
{
    std::wstring rootsFile =
        GetExeDirectory() +
        L"\\orajaPath.txt";

    std::wofstream file(
        rootsFile.c_str());

    if (file)
    {
        file << path;
    }
}

std::wstring LoadBeatorajaPath()
{
    std::wstring rootsFile =
        GetExeDirectory() +
        L"\\orajaPath.txt";

    std::wifstream file(
        rootsFile.c_str());

    if (!file)
    {
        return L"";
    }

    std::wstring path;

    std::getline(
        file,
        path);

    return path;
}

std::wstring SelectFolder()
{
    BROWSEINFOW bi = {};
    bi.lpszTitle =
        L"追加する検索フォルダを選択してください";

    PIDLIST_ABSOLUTE pidl =
        SHBrowseForFolderW(&bi);

    if (!pidl)
    {
        return L"";
    }

    wchar_t path[MAX_PATH];

    if (!SHGetPathFromIDListW(
            pidl,
            path))
    {
        CoTaskMemFree(pidl);
        return L"";
    }

    CoTaskMemFree(pidl);

    return path;
}

std::vector<std::wstring> LoadBmsRoots(
    const std::wstring& configPath)
{
    std::vector<std::wstring> roots;

    std::ifstream file(
        configPath.c_str(),
        std::ios::binary);

    if (!file)
    {
        return roots;
    }

    std::string utf8Content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    std::wstring content =
        Utf8ToWide(
            utf8Content);
    
    if (!content.empty() &&
        content[0] == 0xFEFF)
    {
        content.erase(0, 1);
    }

    size_t rootPos =
        content.find(L"\"bmsroot\"");

    if (rootPos ==
        std::wstring::npos)
    {
        return roots;
    }

    size_t arrayStart =
        content.find(
            L"[",
            rootPos);

    if (arrayStart ==
        std::wstring::npos)
    {
        return roots;
    }

    size_t arrayEnd =
        content.find(
            L"]",
            arrayStart);

    if (arrayEnd ==
        std::wstring::npos)
    {
        return roots;
    }

    std::wstring block =
        content.substr(
            arrayStart,
            arrayEnd - arrayStart);

    bool inString = false;
    std::wstring current;

    for (wchar_t ch : block)
    {
        if (ch == L'"')
        {
            if (inString)
            {
                size_t pos = 0;

                while ((pos =
                    current.find(
                        L"\\\\",
                        pos))
                    != std::wstring::npos)
                {
                    current.replace(
                        pos,
                        2,
                        L"\\");

                    pos += 1;
                }

                roots.push_back(current);

                current.clear();
            }

            inString = !inString;

            continue;
        }

        if (inString)
        {
            current += ch;
        }
    }

    return roots;
}

void AddSearchResults(
    const std::wstring& query,
    std::vector<std::wstring>& files)
{
    Everything_Reset();

    Everything_SetSearchW(
        query.c_str());

    if (!Everything_QueryW(TRUE))
    {
        return;
    }

    wchar_t fullpath[4096];

    DWORD count =
        Everything_GetNumResults();

    for (DWORD i = 0;
        i < count;
        i++)
    {
        Everything_GetResultFullPathNameW(
            i,
            fullpath,
            4096);

        files.push_back(
            fullpath);
    }
}