#include <windows.h>

struct
{
    int     iIndex;
    const wchar_t* szLabel;
    const wchar_t* szDesc;
}
devcaps[] =
{
     HORZSIZE,      L"HORZSIZE",     L"Width in millimeters:",
     VERTSIZE,      L"VERTSIZE",     L"Height in millimeters:",
     HORZRES,       L"HORZRES",      L"Width in pixels:",
     VERTRES,       L"VERTRES",      L"Height in raster lines:",
     BITSPIXEL,     L"BITSPIXEL",    L"Color bits per pixel:",
     PLANES,        L"PLANES",       L"Number of color planes:",
     NUMBRUSHES,    L"NUMBRUSHES",   L"Number of device brushes:",
     NUMPENS,       L"NUMPENS",      L"Number of device pens:",
     NUMMARKERS,    L"NUMMARKERS",   L"Number of device markers:",
     NUMFONTS,      L"NUMFONTS",     L"Number of device fonts:",
     NUMCOLORS,     L"NUMCOLORS",    L"Number of device colors:",
     PDEVICESIZE,   L"PDEVICESIZE",  L"Size of device structure:",
     ASPECTX,       L"ASPECTX",      L"Relative width of pixel:",
     ASPECTY,       L"ASPECTY",      L"Relative height of pixel:",
     ASPECTXY,      L"ASPECTXY",     L"Relative diagonal of pixel:",
     LOGPIXELSX,    L"LOGPIXELSX",   L"Horizontal dots per inch:",
     LOGPIXELSY,    L"LOGPIXELSY",   L"Vertical dots per inch:",
     SIZEPALETTE,   L"SIZEPALETTE",  L"Number of palette entries:",
     NUMRESERVED,   L"NUMRESERVED",  L"Reserved palette entries:",
     COLORRES,      L"COLORRES",     L"Actual color resolution:"
};

constexpr auto NUMLINES{ sizeof (devcaps) / sizeof (devcaps[0]) };

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int  cxChar, cxCaps, cyChar;
    wchar_t     szBuffer[10];
    HDC         hdc;
    int         i;
    PAINTSTRUCT ps;
    TEXTMETRIC  tm;

    switch (message)
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);

        GetTextMetrics(hdc, &tm);
        cxChar = tm.tmAveCharWidth;
        cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
        cyChar = tm.tmHeight + tm.tmExternalLeading;

        ReleaseDC(hwnd, hdc);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        for (i = 0; i < NUMLINES; i++)
        {
            TextOutW(hdc, 0, cyChar * i,
                devcaps[i].szLabel,
                lstrlen(devcaps[i].szLabel));

            TextOutW(hdc, 14 * cxCaps, cyChar * i,
                devcaps[i].szDesc,
                lstrlen(devcaps[i].szDesc));

            SetTextAlign(hdc, TA_RIGHT | TA_TOP);

            TextOutW(hdc, 14 * cxCaps + 35 * cxChar, cyChar * i, szBuffer, wsprintf(szBuffer, L"%5d", GetDeviceCaps(hdc, devcaps[i].iIndex)));

            SetTextAlign(hdc, TA_LEFT | TA_TOP);
        }

        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}


int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int nCmdShow)
{
    static TCHAR szAppName[] = L"DevCaps1";
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    RegisterClass(&wndclass);

    hwnd = CreateWindow(szAppName, TEXT("Device Capabilities"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

