#include <windows.h>
#include <cstdlib>
int cxClient, cyClient;
void DrawRectangle(HWND hwnd)
{
    HBRUSH hBrush;
    HDC    hdc;
    RECT   rect;

    if (cxClient == 0 || cyClient == 0)
        return;
    SetRect(&rect, rand() % cxClient, rand() % cyClient, rand() % cxClient, rand() % cyClient);
    hBrush = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));
    hdc = GetDC(hwnd);
    FillRect(hdc, &rect, hBrush);
    ReleaseDC(hwnd, hdc);
    DeleteObject(hBrush);
}
LRESULT WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)
    {
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, iMsg, wParam, lParam);
}
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("RandRect");
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
    RegisterClassW(&wndclass);
    hwnd = CreateWindowExW(0, szAppName, L"Random Rectangles", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, iCmdShow);
    while (TRUE)
    {
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else
            DrawRectangle(hwnd);
    }
    return static_cast<int>(msg.wParam);
}