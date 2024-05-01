// win32_emoji_draw.cpp --- Win32 emoji drawing sample
// License: MIT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <cassert>
#include <cstdio>

#define WIDTH 600
#define HEIGHT 400
#define CLASSNAME L"Win32 Emoji Draw"
#define FONT_SIZE 80

enum TEXT_VALIGN
{
    TEXT_VALIGN_TOP = 0,
    TEXT_VALIGN_MIDDLE,
    TEXT_VALIGN_BOTTOM,
};

HINSTANCE g_hInst = NULL;
HWND g_hMainWnd = NULL;

/////////////////////////////////////////////////////////////////////////////////////////
// Fill by white?

//#define FILL_BY_WHITE

/////////////////////////////////////////////////////////////////////////////////////////
// Horizontal alignment

//#define HALIGN DT_LEFT // left
#define HALIGN DT_CENTER // center
//#define HALIGN DT_RIGHT // right

/////////////////////////////////////////////////////////////////////////////////////////
// Vertical alignment

//#define VALIGN DT_TOP // top
#define VALIGN DT_VCENTER // middle
//#define VALIGN DT_BOTTOM // bottom

/////////////////////////////////////////////////////////////////////////////////////////

void OnDraw(HWND hwnd, HDC hDC)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

#ifdef FILL_BY_WHITE
    FillRect(hDC, &rc, GetStockBrush(WHITE_BRUSH));
#else
    HBRUSH hbrRed = CreateSolidBrush(RGB(255, 0, 0));
    FillRect(hDC, &rc, hbrRed);
    DeleteObject(hbrRed);
#endif

    // The text and its length
    auto text = L"\U0001F604\U00002665\U0001F4BB 123";
    INT count = lstrlenW(text);

    LOGFONTW lf = { 0 };
    lf.lfHeight = -MulDiv(80, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpynW(lf.lfFaceName, L"Segoe UI Emoji", LF_FACESIZE);

    HFONT hFont = ::CreateFontIndirectW(&lf);
    HGDIOBJ hFontOld = ::SelectObject(hDC, hFont);
    ::SetTextColor(hDC, RGB(255, 255, 255));
    ::SetBkColor(hDC, RGB(0, 0, 0));
    ::SetBkMode(hDC, TRANSPARENT);

    // Calculate the text extent
    UINT uFormat = DT_NOPREFIX | DT_WORDBREAK | HALIGN | VALIGN | DT_CALCRECT;
    RECT rcText = rc;
    DrawTextW(hDC, text, count, &rcText, uFormat);

    // Adjust the text position
    INT cyTextHeight = rcText.bottom - rcText.top;
    rcText = rc;
    if (VALIGN == DT_VCENTER)
        rcText.top = (rc.bottom - cyTextHeight) / 2;
    else if (VALIGN == DT_BOTTOM)
        rcText.top = rc.bottom - cyTextHeight;
    rcText.bottom = rcText.top + cyTextHeight;

    // Draw the text
    uFormat = DT_NOPREFIX | DT_WORDBREAK | HALIGN | VALIGN;
    DrawTextW(hDC, text, count, &rcText, uFormat);

    ::SelectObject(hDC, hFontOld);

    // Draw a rectangle
    rc.left += 10;
    rc.top += 10;
    rc.right -= 10;
    rc.bottom -= 10;
    HPEN hPen = ::CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
    HGDIOBJ hPenOld = ::SelectObject(hDC, hPen);
    ::SelectObject(hDC, GetStockBrush(NULL_BRUSH));
    ::Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    ::SelectObject(hDC, hPenOld);
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    g_hMainWnd = hwnd;
    return TRUE;
}

void OnDestroy(HWND hwnd)
{
    g_hMainWnd = NULL;
    PostQuitMessage(0);
}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hwnd, &ps);
    OnDraw(hwnd, hDC);
    EndPaint(hwnd, &ps);
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
}

BOOL OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    return TRUE;
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    return result;
}

typedef BOOL (WINAPI *FN_SetProcessDpiAwarenessContext)(HANDLE);

INT WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    g_hInst = hInstance;

    // Use manifest for DPI awareness. Period.
#if 0
    auto user32 = GetModuleHandleA("user32.dll");
    auto fn = (FN_SetProcessDpiAwarenessContext)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
    fn(UlongToHandle(-4)); // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#endif

    InitCommonControls();

    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = CLASSNAME;
    if (!RegisterClassExW(&wc))
    {
        MessageBoxA(NULL, "Failed to register window class.", NULL, MB_ICONERROR);
        return -1;
    }

    auto title = L"Win32 Emoji Rendering Demo";
    auto style = WS_OVERLAPPEDWINDOW;
    auto exstyle = 0;
    RECT rc = { 0, 0, WIDTH, HEIGHT };
    BOOL bMenu = FALSE;
    AdjustWindowRectEx(&rc, style, bMenu, exstyle);
    INT cx = rc.right - rc.left, cy = rc.bottom - rc.top;
    HWND hwnd = CreateWindowW(CLASSNAME, title, style,
                              CW_USEDEFAULT, CW_USEDEFAULT, cx, cy,
                              NULL, NULL, hInstance, NULL);
    if (!hwnd)
    {
        MessageBoxA(NULL, "Failed to create main window.", NULL, MB_ICONERROR);
        return -2;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (INT)msg.wParam;
}
