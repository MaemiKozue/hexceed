#include "program.hpp"

#include <vector>
#include <string>
#include <iostream>

#include <windows.h>

#include <chrono>
#include <thread>


static std::wstring wstr(std::string s)
{
    std::wstring res = L"";

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


// Assuming input from default camera
Program::Program(std::string name) :
    name(name),
    capture(0),
    cap_method(CAPTURE_CAMERA)
{
    this->capture.set(cv::CAP_PROP_FRAME_WIDTH,  1920);
    this->capture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
}


static bool fill_bitmap_info (HDC dc, HBITMAP bitmap, BITMAPINFO &bmi)
{
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    // Query the bitmap to fill automatically some information
    int rci = GetDIBits(dc, bitmap, 0, 0, NULL, &info, DIB_RGB_COLORS);
    switch (rci)
    {
        case 0:
        {
            std::cerr << "Unsuccessful getting bitmap info" << std::endl;
            return false;
            break;
        }
        case ERROR_INVALID_PARAMETER:
        {
            std::cerr << "Error: " << "ERROR_INVALID_PARAMETER" << std::endl;
            return false;
            break;
        }
        default:
        {
            // std::cout << "Success getting bitmap info: " << rci << std::endl;
            break;
        }
    }

    // Some specific information
    info.bmiHeader.biCompression = BI_RGB;

    // Windows bitmaps are bottom up, and negative length flips it upside down
    info.bmiHeader.biHeight = - info.bmiHeader.biHeight;

    // Copy
    bmi = info;

    return true;
}


static bool fill_image_from_bitmap (HDC dc, HBITMAP bitmap, BITMAPINFO bmi, char *img)
{
    bool success = false;
    int rci = GetDIBits(dc, bitmap, 0, bmi.bmiHeader.biHeight, img, &bmi, DIB_RGB_COLORS);
    switch (rci)
    {
        case 0:
        {
            std::cerr << "Unsuccessful copying bitmap" << std::endl;
            success = false;
            break;
        }
        case ERROR_INVALID_PARAMETER:
        {
            std::cerr << "Error: " << "ERROR_INVALID_PARAMETER" << std::endl;
            success = false;
            break;
        }
        default:
        {
            // std::cout << "Success copying bitmap: " << rci << std::endl;
            success = true;
            break;
        }
    }
    return success;
}


static size_t bitmap_step (BITMAPINFO bmi)
{
    int w = bmi.bmiHeader.biWidth;
    int b = bmi.bmiHeader.biBitCount;
    int s = sizeof(DWORD);
    size_t step = (w * b + (s * 8) - 1) / (s * 8) * s;
    return step;
}


static Image gdi_capture(std::string name)
{
    // Retrieve handle on window
    std::wstring wsname = wstr(name);
    HWND hWndProgram = FindWindowExW(NULL, NULL, NULL, wsname.c_str());
    if (hWndProgram == NULL) {
        std::cerr << "Cannot find window with name '" << name << "'" << std::endl;
        print_visible_windows(std::cout);
        return Image();
    }

    // Retrieve device context
    HDC dc = GetDC(hWndProgram);
    if (dc == NULL) {
        std::cerr << "Cannot get device context for " << name << std::endl;
        return Image();
    }

    // Retrieve frame dimensions
    RECT rect;
    bool rc = GetClientRect(hWndProgram, &rect);
    if (!rc) {
        std::cerr << "Cannot get window dimensions" << std::endl;
        return Image();
    }

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create a copy context and copy the frame
    HDC cdc = CreateCompatibleDC(dc);
    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(dc, width, height);
    HGDIOBJ default_bitmap = SelectObject(cdc, hCaptureBitmap);

    BitBlt(cdc, 0, 0, width, height, dc, 0, 0, SRCCOPY);

    // We don't need the source device context anymore
    ReleaseDC(hWndProgram, dc);

    // Preparing the exportation of the frame as bitmap
    Image im;
    BITMAPINFO bmi = {};
    // Filling bitmap header
    if (!fill_bitmap_info(cdc, hCaptureBitmap, bmi)) {
        std::cerr << "Failed to fill bitmap info" << std::endl;
    }
    else {
        // Allocating space for output
        size_t im_size = bmi.bmiHeader.biSizeImage * sizeof(char);
        char *img = static_cast<char*>(malloc(im_size));

        if (!fill_image_from_bitmap(cdc, hCaptureBitmap, bmi, img)) {
            std::cerr << "Failed to create bitmap" << std::endl;
        }
        else {
            // Rows of windows bitmaps are aligned on DWORD
            size_t step = bitmap_step(bmi);
            im = Image(img, im_size, width, height, step);
        }
    }

    // Releasing copy ressources
    SelectObject(cdc, default_bitmap);
    DeleteObject(hCaptureBitmap);
    DeleteDC(cdc);

    return im;
}


cv::Mat Program::screenshot()
{
    cv::Mat ss;

    if (this->cap_method == CAPTURE_GDI) {
        Image im;
        im = gdi_capture(this->name);
        if (im.get() != NULL) {
            ss = cv::Mat(im.height, im.width, CV_8UC4, im.get());
        }
    }
    else if (this->cap_method == CAPTURE_CAMERA) {
        this->capture >> ss;
    }
    else {
        std::cerr << "Capture not implemented" << std::endl;
    }

    return ss;
}


static bool win_send_input(std::string window_name, Input in)
{
    if (in.getType() == MOUSE && in.getButton() == MB_LEFT) {
        std::wstring wname = wstr(window_name);
        HWND hWindow = FindWindowExW(NULL, NULL, NULL, wname.c_str());
        if (hWindow == NULL) {
            std::cerr << "Cannot find program " << window_name << std::endl;
            return false;
        }
        SetForegroundWindow(hWindow);
        // // WPARAM wParam = MK_LBUTTON;
        // WPARAM wParam = 0;
        // LPARAM lParam = (in.getY() << 16) | (in.getX() & 0xFFFF);
        // PostMessageW(hWindow, WM_MOUSEMOVE, wParam, lParam);
        // PostMessageW(hWindow, WM_LBUTTONDOWN, wParam, lParam);
        // // PostMessageW(hWindow, WM_LBUTTONUP, wParam, lParam);
        // return true;
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi = {};
        input.mi.dx = (LONG) (((double) in.getX() / 1920) * 65535);
        input.mi.dy = (LONG) (((double) in.getY() / 1080) * 65535);
        input.mi.mouseData = 0;
        input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        input.mi.time = 0;
        input.mi.dwExtraInfo = NULL;
        int rc = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        rc += SendInput(1, &input, sizeof(INPUT));
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE;
        rc += SendInput(1, &input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE;
        rc += SendInput(1, &input, sizeof(INPUT));

        return rc >= 1;
    }
    else {
        std::cerr << "Input not implemented: " << in << std::endl;
        return false;
    }
}


bool Program::input(Input in) const
{
    std::cout << "Sending " << in << " to " << this->name << std::endl;
    return win_send_input(this->name, in);
}


void Program::set_method(capture_method method)
{
    this->cap_method = method;
}
