#include <windows.h>

void GetScale(double* xScale, double* yScale)
{
    DEVMODE   dm;
    dm.dmSize = sizeof(DEVMODE);
    EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm);
    *xScale = dm.dmPelsWidth / (double)GetSystemMetrics(SM_CXSCREEN);
    *yScale = dm.dmPelsHeight / (double)GetSystemMetrics(SM_CYSCREEN);

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int  cxClient, cyClient, cxSource, cySource;
    HDC         hdcClient, hdcWindow;
    int         x, y;
    PAINTSTRUCT ps;
    double xScale, yScale;
    switch (message)
    {
    case WM_CREATE:
        
        GetScale(&xScale, &yScale);
        cxSource = (GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXSMICON))*xScale;
        cySource = (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION))*yScale;


        return 0;
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;
    case WM_PAINT:
        hdcClient = BeginPaint(hwnd, &ps);
        hdcWindow = GetWindowDC(hwnd);
        for (y = 0; y < cyClient; y += cySource)
            for (x = 0; x < cxClient; x += cxSource)
            {
                BitBlt(hdcClient, x, y, cxSource, cySource, hdcWindow, 0, 0, SRCCOPY);
            }
        ReleaseDC(hwnd, hdcWindow);
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("BitBlt");
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_INFORMATION);
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
    hwnd = CreateWindowExW(0L, szAppName, TEXT("BitBlt Demo"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}