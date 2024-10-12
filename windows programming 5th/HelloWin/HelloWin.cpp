//本程序创建一个窗口，并显示文本
#include <windows.h>
#include <mmSystem.h>
#pragma comment(lib,"winmm.lib") //用于语音播放
//消息处理函数
LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rect;
    switch (message)
    {
    case WM_CREATE:
        PlaySoundW(L"hellowin.wav", NULL, SND_FILENAME | SND_ASYNC);
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rect);
        DrawTextW(hdc, L"Hello, Windows 11!", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
//主函数
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int nCmdShow)
{
    static TCHAR szAppName[] = L"HelloWin";
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
    RegisterClassW(&wndclass);
    hwnd = CreateWindowExW(0L, szAppName, L"The Hello Program", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}