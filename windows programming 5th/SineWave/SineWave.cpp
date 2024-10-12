//»æÖÆÕýÏÒÇúÏß

#include <windows.h>
#include <cmath>

constexpr auto NUM = 1000000;
constexpr auto TWOPI = (2 * 3.14159265358979);

POINT apt[NUM];

LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int  cxClient, cyClient;
    HDC         hdc;
    int         i;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        MoveToEx(hdc, 0, cyClient / 2, NULL);
        LineTo(hdc, cxClient, cyClient / 2);

        for (i = 0; i < NUM; i++)
        {
            apt[i].x = i * cxClient / NUM;
            apt[i].y = static_cast<int>(cyClient / 2 * (1 - sin(TWOPI * i / NUM)));
        }

        Polyline(hdc, apt, NUM);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("SineWave");
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
    hwnd = CreateWindow(szAppName, TEXT("Sine Wave Using Polyline"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, iCmdShow);
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}


