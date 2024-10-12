#include <windows.h>

LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int  cxClient, cyClient;
    HDC         hdc;
    PAINTSTRUCT ps;
    switch (message)
    {
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        Rectangle(hdc, cxClient / 8, cyClient / 8, 7 * cxClient / 8, 7 * cyClient / 8);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, cxClient, cyClient);
        MoveToEx(hdc, 0, cyClient, NULL);
        LineTo(hdc, cxClient, 0);
        Ellipse(hdc, cxClient / 8, cyClient / 8, 7 * cxClient / 8, 7 * cyClient / 8);
        RoundRect(hdc, cxClient / 4, cyClient / 4, 3 * cxClient / 4, 3 * cyClient / 4, cxClient / 4, cyClient / 4);
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    static TCHAR szAppName[] = L"LineDemo";
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    RegisterClass(&wndclass);
    hwnd = CreateWindow(szAppName, L"Line Demonstration", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, iCmdShow);
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

