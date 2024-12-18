#include <windows.h>

LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static POINT aptFigure[10] = { 10, 70, 50, 70, 50, 10, 90, 10, 90, 50, 30, 50, 30, 90, 70, 90, 70, 30, 10, 30 };
    static int   cxClient, cyClient;
    HDC          hdc;
    int          i;
    PAINTSTRUCT  ps;
    POINT        apt[10];

    switch (message)
    {
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        SelectObject(hdc, GetStockObject(GRAY_BRUSH));

        for (i = 0; i < 10; i++)
        {
            apt[i].x = cxClient * aptFigure[i].x / 200;
            apt[i].y = cyClient * aptFigure[i].y / 100;
        }

        SetPolyFillMode(hdc, ALTERNATE);
        Polygon(hdc, apt, 10);

        for (i = 0; i < 10; i++)
        {
            apt[i].x += cxClient / 2;
        }

        SetPolyFillMode(hdc, WINDING);
        Polygon(hdc, apt, 10);

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
    static TCHAR szAppName[] = TEXT("AltWind");
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
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    RegisterClassW(&wndclass);
    hwnd = CreateWindowExW(0, szAppName, TEXT("Bezier Splines"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, iCmdShow);
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}