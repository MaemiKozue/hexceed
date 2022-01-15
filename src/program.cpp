#include "program.hpp"

#include <vector>
#include <string>
#include <codecvt>
#include <iostream>

#include <windows.h>


static std::wstring wstr(std::string s)
{
    // std::wstring_convert<std::codecvt<wchar_t, >> converter;
    std::wstring res = L"";
    
    // res = converter.from_bytes(s);
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len <= 0) {
        std::cerr << "Error converting: '" << s << "'" << std::endl;
    }
    else {
        res.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, res.data(), len);
    }
    
    return res;
}

static std::string unwstr(std::wstring s)
{
    std::string res = "";
    int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 0) {
        std::wcerr << L"Error converting' " << s << L"'" << std::endl;
    }
    else {
        res.resize(len);
        WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, res.data(), len, NULL, NULL);
    }
    return res;
}

static BOOL CALLBACK list_names(HWND hwnd, LPARAM plist)
{
    std::vector<std::string> &lst = *reinterpret_cast<std::vector<std::string>*>(plist);
    std::wstring name = L"";
    int len = GetWindowTextLengthW(hwnd);
    name.resize(len);
    GetWindowTextW(hwnd, name.data(), len+1);

    // Filter out invisible windows
    if (!IsWindowVisible(hwnd) || name.size() <= 0) {
        return true;
    }

    lst.push_back(unwstr(name));
    return true;
}

static void print_visible_windows(std::ostream &out)
{
    std::vector<std::string> win_list;
    bool rc = EnumWindows(list_names, reinterpret_cast<LPARAM>(&win_list));
    if (!rc) {
        std::cerr << "Cannot enumerate windows" << std::endl;
    }
    else {
        std::cout << "Windows list:" << std::endl; 
        for (std::string wname : win_list) {
            out << " - " << wname << std::endl; 
        }
    }
}

static HWND window;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    hwnd;
    uMsg;
    wParam;
    lParam;
    std::cout << "In callback(" << uMsg << ")" << std::endl;
    Program *prog = NULL;
    switch (uMsg) {
        // case WM_GETMINMAXINFO: {
        //     std::cout << "WM_GETMINMAXINFO" << std::endl;
        //     return false;
        //     break;
        // }
        // case WM_NCCALCSIZE: {
        //     std::cout << "WM_NCCALCSIZE" << std::endl;
        //     return false;
        //     break;
        // }
        // case WM_CREATE: {
        //     std::cout << "WM_CREATE" << std::endl;
        //     return false;
        //     break;
        // }
        case WM_NCCREATE: {
            std::cout << "WM_NCCREATE" << std::endl;
            std::cout << "Setup" << std::endl;
            CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            prog = reinterpret_cast<Program*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(prog));
            return true;
            break;
        }
        default: {
            std::cout << "default" << std::endl;
            LONG_PTR pProg = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            if (pProg != NULL) {
                prog = reinterpret_cast<Program*>(pProg);
                prog->screenshot();
                UpdateWindow(hwnd);
            }
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
            break;
        }
        // default: {
        //     return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        // }
    }
}

void Program::loop() const
{
    MSG msg;
    BOOL rb;
    std::cout << "preloop" << std::endl;
    while ((rb = GetMessageW(&msg, NULL, 0, 0)) != 0)
    {
        std::cout << "loop" << std::endl;
        if (rb == -1) {
            std::cerr << "Error while getting message" << std::endl;
            return;
        }
        else {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    std::cout << "outloop" << std::endl;
}


Program::Program(std::string name) :
    name(name)
{
    HINSTANCE instance = GetModuleHandleW(NULL);

    WNDCLASSEXW cls = {};
    cls.cbSize = sizeof(WNDCLASSEXW);
    cls.lpfnWndProc = WindowProc;
    cls.hInstance = instance;
    cls.lpszClassName = L"hexceed_solver";
    RegisterClassExW(&cls);
    std::cout << "Creating window" << std::endl;
    HWND win = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW, 
        L"hexceed_solver",
        L"An hexceed solver",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, // location
        CW_USEDEFAULT, CW_USEDEFAULT, // dimensions
        NULL, NULL, instance, this
    );
    std::cout << "End constructor" << std::endl;
    ShowWindow(win, SW_SHOW);
    window = win;
}

Program::~Program()
{
}


Image Program::screenshot() const
{
    std::cout << "Taking a screenshot from " << this->name << std::endl;
    std::wstring wsname = wstr(this->name);
    HWND hWndProgram = FindWindowExW(NULL, NULL, NULL, wsname.c_str());
    if (hWndProgram == NULL) {
        std::cerr << "Cannot find window with name '" << this->name << "'" << std::endl;
        print_visible_windows(std::cout);
    }
    else {
        std::cout << "Found window" << std::endl;
        HDC dc = GetDC(hWndProgram);
        if (dc == NULL) {
            std::cerr << "Cannot get device context for " << this->name << std::endl;
            return NULL;
        }

        RECT rect;
        bool rc = GetClientRect(hWndProgram, &rect);
        if (!rc) {
            std::cerr << "Cannot get window dimensions" << std::endl;
            return NULL;
        }
        
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        // HDC cdc = CreateCompatibleDC(dc);
        // HBITMAP hCaptureBitmap = CreateCompatibleBitmap(dc, width, height);
        
        HWND win = window;
        BitBlt(GetDC(win), 0, 0, width, height, dc, 0, 0, SRCCOPY);
        ReleaseDC(hWndProgram, dc);
    }
    return NULL;
}

bool Program::input(Input in) const
{
    std::cout << "Sending " << in << " to " << this->name << std::endl;
    return false;
}