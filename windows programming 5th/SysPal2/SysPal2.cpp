#include <windows.h>

LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

TCHAR szAppName[] = TEXT("SysPal2");

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    HWND     hwnd;
    MSG      msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, TEXT("System Palette #2"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
        return 0;

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HPALETTE hPalette;
    static int      cxClient, cyClient;
    HBRUSH          hBrush;
    HDC             hdc;
    int             i, x, y;
    LOGPALETTE* plp;
    PAINTSTRUCT     ps;
    RECT            rect;

    switch (message)
    {
    case WM_CREATE:
        plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY));
        plp->palVersion = 0x0300;
        plp->palNumEntries = 256;
        for (i = 0; i < 256; i++)
        {
            plp->palPalEntry[i].peRed = i;
            plp->palPalEntry[i].peGreen = 0;
            plp->palPalEntry[i].peBlue = 0;
            plp->palPalEntry[i].peFlags = PC_EXPLICIT;
        }
        hPalette = CreatePalette(plp);
        free(plp);
        return 0;
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        SelectPalette(hdc, hPalette, FALSE);
        RealizePalette(hdc);
        for (y = 0; y < 16; y++)
            for (x = 0; x < 16; x++)
            {
                hBrush = CreateSolidBrush(PALETTEINDEX(16 * y + x));
                SetRect(&rect, x * cxClient / 16, y * cyClient / 16,
                    (x + 1) * cxClient / 16, (y + 1) * cyClient / 16);
                FillRect(hdc, &rect, hBrush);
                DeleteObject(hBrush);
            }
        EndPaint(hwnd, &ps);
        return 0;

    case WM_PALETTECHANGED:
        if ((HWND)wParam != hwnd)
            InvalidateRect(hwnd, NULL, FALSE);

        return 0;

    case WM_DESTROY:
        DeleteObject(hPalette);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
