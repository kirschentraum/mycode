#include <windows.h>
TCHAR szAppName[] = TEXT("SysPal1");
LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int   cxClient, cyClient;
    static SIZE  sizeChar;
    HDC          hdc;
    int          i, x, y;
    PAINTSTRUCT  ps;
    PALETTEENTRY pe[256];
    TCHAR        szBuffer[16];
    switch (message)
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);
        SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
        GetTextExtentPoint32W(hdc, TEXT("FF-FF-FF"), 10, &sizeChar);
        ReleaseDC(hwnd, hdc);
        return 0;
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
        GetSystemPaletteEntries(hdc, 0, 256, pe);
        for (i = 0, x = 0, y = 0; i < 256; i++)
        {
            wsprintf(szBuffer, TEXT("%02X-%02X-%02X"),
                pe[i].peRed, pe[i].peGreen, pe[i].peBlue);

            TextOut(hdc, x, y, szBuffer, lstrlen(szBuffer));

            if ((x += sizeChar.cx) + sizeChar.cx > cxClient)
            {
                x = 0;

                if ((y += sizeChar.cy) > cyClient)
                    break;
            }
        }
        EndPaint(hwnd, &ps);
        return 0;
    case WM_PALETTECHANGED:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    HWND     hwnd;
    MSG      msg;
    WNDCLASSW wndclass;
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

    if (!RegisterClassW(&wndclass))
    {
        MessageBoxW(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindowW(szAppName, TEXT("System Palette #1"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
        return 0;

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}


