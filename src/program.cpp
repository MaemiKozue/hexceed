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
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
            break;
        }
        case WM_NCCREATE: {
            std::cout << "WM_NCCREATE" << std::endl;
            std::cout << "Setup" << std::endl;
            CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            prog = reinterpret_cast<Program*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(prog));
            return TRUE;
            break;
        }
        case WM_PAINT: {
            std::cout << "Drawing" << std::endl;
            LONG_PTR pProg = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            if (pProg != NULL) {
                prog = reinterpret_cast<Program*>(pProg);
                prog->screenshot();
            }
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
            break;
        }
        default: {
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
            break;
        }
    }
}

void Program::loop() const
{
    bool quit = false;
    MSG msg;
    BOOL rb;
    clock_t t_prev = clock();
    while (!quit)
    {
        rb = PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE);
        if (rb == 0) {
            clock_t t_now = clock();
            clock_t delta_time = t_now - t_prev;
            if (delta_time > 1./120) {
                InvalidateRect(window, NULL, TRUE);
                t_prev = t_now;
            }
        }
        else {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);

            if (msg.message == WM_QUIT) {
                std::cout << "Quitting" << std::endl;
                quit = true;
            }
        }
    }
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

    HWND win = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW, 
        L"hexceed_solver",
        L"An hexceed solver",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, // location
        CW_USEDEFAULT, CW_USEDEFAULT, // dimensions
        NULL, NULL, instance, this
    );

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
            return Image();
        }

        RECT rect;
        bool rc = GetClientRect(hWndProgram, &rect);
        if (!rc) {
            std::cerr << "Cannot get window dimensions" << std::endl;
            return Image();
        }
        
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        HWND win = window;
        HDC cdc = CreateCompatibleDC(dc);
        HBITMAP hCaptureBitmap = CreateCompatibleBitmap(dc, width, height);
        HGDIOBJ default_bitmap = SelectObject(cdc, hCaptureBitmap);
        BitBlt(cdc, 0, 0, width, height, dc, 0, 0, SRCCOPY);

        BitBlt(GetDC(win), 0, 0, width, height, dc, 0, 0, SRCCOPY);
        ReleaseDC(win, dc);
        ReleaseDC(hWndProgram, dc);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        BITMAPINFOHEADER bmi_header;
        int rci;
        rci = GetDIBits(cdc, hCaptureBitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS);

        switch (rci)
        {
            case 0:
            {
                std::cerr << "Unsuccessful getting bits" << std::endl;
                break;
            }
            case ERROR_INVALID_PARAMETER:
            {
                std::cerr << "Error: " << "ERROR_INVALID_PARAMETER" << std::endl;
                break;
            }        
            default: 
            {
                std::cout << "Success getting bits: " << rci << std::endl;
                break;
            }
        }
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biHeight = - bmi.bmiHeader.biHeight;
        bmi_header = bmi.bmiHeader;
        std::cout << "Size: " << bmi_header.biSizeImage << std::endl;
        char *img = ((char*)malloc(bmi_header.biSizeImage));
        rci = GetDIBits(cdc, hCaptureBitmap, 0, bmi.bmiHeader.biHeight, img, &bmi, DIB_RGB_COLORS);
        std::cout << "rci: " << rci << std::endl;
        SelectObject(cdc, default_bitmap);
        DeleteObject(hCaptureBitmap);
        DeleteDC(cdc);

        size_t step = (width * bmi.bmiHeader.biBitCount + (sizeof(DWORD) * 8) - 1) / (sizeof(DWORD) * 8) * sizeof(DWORD);
        return Image(img, width, height, step);
    }
    return Image();
}

bool Program::input(Input in) const
{
    std::cout << "Sending " << in << " to " << this->name << std::endl;
    return false;
}