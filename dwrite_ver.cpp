// win32_emoji_draw.cpp --- Win32 emoji drawing sample
// License: MIT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <cassert>
#include <cstdio>

#define WIDTH 600
#define HEIGHT 480
#define CLASSNAME L"Win32 Emoji Draw"
#define FONT_SIZE 80

enum TEXT_VALIGN
{
    TEXT_VALIGN_TOP = 0,
    TEXT_VALIGN_MIDDLE,
    TEXT_VALIGN_BOTTOM,
};

struct DWriteSupport
{
    ID2D1Factory *d2d_factory = NULL;
    IDWriteFactory *dwrite_factory = NULL;
    ID2D1HwndRenderTarget *render_target = NULL;
    ID2D1SolidColorBrush *white_brush = NULL;
    ID2D1SolidColorBrush *black_brush = NULL;
    FLOAT m_width = WIDTH;
    FLOAT m_height = HEIGHT;

    DWriteSupport()
    {
    }

    ~DWriteSupport()
    {
        ShutDown();
    }

    HRESULT Setup(HWND hwnd)
    {
        HRESULT hr;
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
        assert(hr == S_OK);
        if (FAILED(hr))
            return hr;

        D2D1_SIZE_U size = { UINT32(m_width), UINT32(m_height) };
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
        hr = d2d_factory->CreateHwndRenderTarget(props, D2D1::HwndRenderTargetProperties(hwnd, size), &render_target);
        assert(hr == S_OK);
        if (FAILED(hr))
            return hr;

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite_factory));
        assert(hr == S_OK);
        if (FAILED(hr))
            return hr;

        hr = render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &white_brush);
        assert(hr == S_OK);
        if (FAILED(hr))
            return hr;

        hr = render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &black_brush);
        assert(hr == S_OK);
        if (FAILED(hr))
            return hr;

        return hr;
    }

    void ShutDown()
    {
        if (d2d_factory)
        {
            d2d_factory->Release();
            d2d_factory = NULL;
        }
        if (dwrite_factory)
        {
            dwrite_factory->Release();
            dwrite_factory = NULL;
        }
        if (render_target)
        {
            render_target->Release();
            render_target = NULL;
        }
        if (white_brush)
        {
            white_brush->Release();
            white_brush = NULL;
        }
        if (black_brush)
        {
            black_brush->Release();
            black_brush = NULL;
        }
    }

    D2D1_SIZE_F GetSize()
    {
        assert(render_target);
        return render_target->GetSize();
    }

    D2D1_SIZE_F GetTextExtent(IDWriteTextFormat *text_format, LPCWSTR text, INT count, FLOAT cx, FLOAT cy)
    {
        IDWriteTextLayout *write_text_layout = NULL;
        dwrite_factory->CreateTextLayout(text, count, text_format, cx, cy, &write_text_layout);
        if (!write_text_layout)
            return { 0, 0 };

        DWRITE_TEXT_METRICS metrics;
        write_text_layout->GetMetrics(&metrics);

        if (write_text_layout)
        {
            write_text_layout->Release();
            write_text_layout = NULL;
        }

        return { metrics.width, metrics.height };
    }

    void DrawText(
        IDWriteTextFormat *text_format,
        LPCWSTR text,
        INT count,
        ID2D1SolidColorBrush *brush,
        D2D1_RECT_F rect,
        DWRITE_TEXT_ALIGNMENT halign = DWRITE_TEXT_ALIGNMENT_LEADING,
        TEXT_VALIGN valign = TEXT_VALIGN_TOP,
        BOOL color_font = TRUE)
    {
        assert(text_format);
        text_format->SetTextAlignment(halign);

        switch (valign)
        {
        case TEXT_VALIGN_TOP:
            break;
        case TEXT_VALIGN_MIDDLE:
        case TEXT_VALIGN_BOTTOM:
            // Adjust vertical position
            FLOAT cx = rect.right - rect.left, cy = rect.bottom - rect.top;
            D2D1_SIZE_F text_extent = GetTextExtent(text_format, text, count, cx, cy);
            if (valign == TEXT_VALIGN_MIDDLE)
                rect.top = (cy - text_extent.height) / 2;
            else
                rect.top = cy - text_extent.height;
            rect.bottom = rect.top + text_extent.height;
            break;
        }

        assert(brush);
        auto flags = (color_font ? D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE);
        render_target->DrawText(text, count, text_format, rect, brush, flags);
    }

    FLOAT ConvertPointSizeToDIP(FLOAT points) const
    {
        return (points / 72.0f) * 96.0f;
    };
};

HINSTANCE g_hInst = NULL;
HWND g_hMainWnd = NULL;
DWriteSupport g_dwrite;

/////////////////////////////////////////////////////////////////////////////////////////
// Fill by white?

//#define FILL_BY_WHITE

/////////////////////////////////////////////////////////////////////////////////////////
// Color font or not

#define COLOR_FONT TRUE
//#define COLOR_FONT FALSE

/////////////////////////////////////////////////////////////////////////////////////////
// Horizontal alignment

//#define HALIGN DWRITE_TEXT_ALIGNMENT_LEADING // left
#define HALIGN DWRITE_TEXT_ALIGNMENT_CENTER // center
//#define HALIGN DWRITE_TEXT_ALIGNMENT_TRAILING // right

/////////////////////////////////////////////////////////////////////////////////////////
// Vertical alignment

//#define VALIGN TEXT_VALIGN_TOP // top
#define VALIGN TEXT_VALIGN_MIDDLE // middle
//#define VALIGN TEXT_VALIGN_BOTTOM // bottom

/////////////////////////////////////////////////////////////////////////////////////////

void OnDraw(HWND hwnd, ID2D1HwndRenderTarget *render_target)
{
    // Start rendering
    render_target->BeginDraw();

    // Calculate the client area
    D2D1_RECT_F rect = D2D1::RectF(0, 0, g_dwrite.m_width, g_dwrite.m_height);

    // Fill background
#ifdef FILL_BY_WHITE
    render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));
#else
    render_target->Clear(D2D1::ColorF(255, 0, 0, 1.0)); // Red
#endif

    // Create a text format
    HRESULT hr;
    IDWriteTextFormat *text_format = NULL;
    hr = g_dwrite.dwrite_factory->CreateTextFormat(L"Segoe UI Emoji",
                                                   NULL,
                                                   DWRITE_FONT_WEIGHT_NORMAL,
                                                   DWRITE_FONT_STYLE_NORMAL,
                                                   DWRITE_FONT_STRETCH_NORMAL,
                                                   g_dwrite.ConvertPointSizeToDIP(FONT_SIZE),
                                                   L"",
                                                   &text_format);
    assert(hr == S_OK);

    if (SUCCEEDED(hr))
    {
        // The text and its length
        auto text = L"\U0001F604\U00002665\U0001F4BB 123";
        INT count = lstrlenW(text);

        // Draw the text
        g_dwrite.DrawText(text_format, text, count, g_dwrite.white_brush, rect,
                          HALIGN, VALIGN, COLOR_FONT);
    }

    // Release text format
    if (text_format)
    {
        text_format->Release();
        text_format = NULL;
    }

    // Draw a rectangle
    rect.left += 10;
    rect.top += 10;
    rect.right -= 10;
    rect.bottom -= 10;
    render_target->DrawRectangle(rect, g_dwrite.white_brush, 3, NULL);

    // End rendering
    render_target->EndDraw();
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    g_hMainWnd = hwnd;
    if (g_dwrite.Setup(hwnd) != S_OK)
        return FALSE;

    return TRUE;
}

void OnDestroy(HWND hwnd)
{
    g_dwrite.ShutDown();
    g_hMainWnd = NULL;
    PostQuitMessage(0);
}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    OnDraw(hwnd, g_dwrite.render_target);
    EndPaint(hwnd, &ps);
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (!g_dwrite.render_target)
        return;

    RECT rc;
    GetClientRect(hwnd, &rc);
    g_dwrite.m_width = rc.right;
    g_dwrite.m_height = rc.bottom;
    g_dwrite.render_target->Resize(D2D1_SIZE_U{ UINT32(rc.right), UINT32(rc.bottom) });
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
    HWND hwnd = CreateWindowW(CLASSNAME, title, WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
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
