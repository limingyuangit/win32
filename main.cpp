#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")

// 原始数据
std::vector<std::wstring> g_data = {
    L"Apple", L"Banana", L"Cherry", L"Date", L"Elderberry",
    L"Fig", L"Grape", L"Honeydew", L"Kiwi", L"Lemon",
    L"Mango", L"Nectarine", L"Orange", L"Papaya", L"Quince"
};

// 过滤后指向原始数据的索引
std::vector<int> g_filtered;  
HWND g_hEdit, g_hList;
std::wstring g_filterText;    // 当前过滤文本（大写）

// 将字符串转为大写 (简易版，不处理特殊 Unicode)
std::wstring ToUpper(const std::wstring& s) {
    std::wstring up(s.length(), L'\0');
    for (size_t i = 0; i < s.length(); ++i)
        up[i] = towupper(s[i]);
    return up;
}

// 执行过滤，更新 g_filtered
void ApplyFilter() {
    g_filtered.clear();
    if (g_filterText.empty()) {
        for (int i = 0; i < (int)g_data.size(); ++i)
            g_filtered.push_back(i);
    } else {
        for (int i = 0; i < (int)g_data.size(); ++i) {
            if (ToUpper(g_data[i]).find(g_filterText) != std::wstring::npos)
                g_filtered.push_back(i);
        }
    }
    // 通知列表视图项数改变，并重绘
    ListView_SetItemCount(g_hList, g_filtered.size());
    InvalidateRect(g_hList, NULL, TRUE);
}

// 虚拟列表的显示回调
LRESULT OnListGetDispInfo(NMLVDISPINFO* pdi) {
    if (pdi->item.mask & LVIF_TEXT) {
        int idx = pdi->item.iItem;          // 虚拟索引
        if (idx >= 0 && idx < (int)g_filtered.size()) {
            int realIdx = g_filtered[idx];  // 对应原始数据索引
            const std::wstring& text = g_data[realIdx];
            lstrcpyn(pdi->item.pszText, text.c_str(), pdi->item.cchTextMax);
        }
    }
    return 0;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // 创建 Edit
            g_hEdit = CreateWindowW(L"EDIT", L"", 
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                10, 10, 260, 25, hWnd, NULL, NULL, NULL);
            // 创建 ListView (虚拟列表 + 报告视图 + 单列)
            g_hList = CreateWindowW(WC_LISTVIEWW, L"",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL,
                10, 45, 260, 200, hWnd, NULL, NULL, NULL);
            // 添加列
            LVCOLUMNW lvc = { LVCF_WIDTH | LVCF_TEXT, 0, 240, L"Fruit" };
            ListView_InsertColumn(g_hList, 0, &lvc);
            // 初始显示全部
            ApplyFilter();
            return 0;
        }
        case WM_SIZE: {
            int width = LOWORD(lParam), height = HIWORD(lParam);
            SetWindowPos(g_hEdit, NULL, 10, 10, width - 20, 25, SWP_NOZORDER);
            SetWindowPos(g_hList, NULL, 10, 45, width - 20, height - 55, SWP_NOZORDER);
            return 0;
        }
        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == g_hEdit) {
                // 获取编辑框文本并转大写
                wchar_t buf[256];
                GetWindowTextW(g_hEdit, buf, 256);
                g_filterText = ToUpper(buf);
                // 执行过滤（防抖可自行加定时器，此处为简单演示）
                ApplyFilter();
            }
            return 0;
        }
        case WM_NOTIFY: {
            NMHDR* pnmh = (NMHDR*)lParam;
            if (pnmh->idFrom == (UINT_PTR)g_hList && pnmh->code == LVN_GETDISPINFOW) {
                return OnListGetDispInfo((NMLVDISPINFO*)lParam);
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 入口点
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    InitCommonControls();  // 加载通用控件样式
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"FilterListClass";
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowW(L"FilterListClass", L"Win32 过滤列表", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        300, 300, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
