#include <windows.h>

constexpr auto ID_EDIT = 1;
TCHAR szAppName[] = L"PopPad1";

LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndEdit;
    switch (message)
    {
    case WM_CREATE:
        hwndEdit=CreateWindowW(L"edit", NULL, WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL, 0, 0, 0, 0, hwnd, (HMENU)ID_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        return 0;
    case WM_SETFOCUS:
        SetFocus(hwndEdit);
        return 0;
    case WM_SIZE:
        MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_EDIT)
            if (HIWORD(wParam) == EN_ERRSPACE || HIWORD(wParam) == EN_MAXTEXT)
                MessageBoxW(hwnd, L"EDIT control out of space.", szAppName, MB_OK);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR szCmdLine, _In_ int iCmdShow)
{
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = CreateSolidBrush(0);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    if (!RegisterClassW(&wndclass))
    {
        MessageBoxW(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
        return 0;
    }
    hwnd = CreateWindowW(szAppName, TEXT("Color Scroll"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}