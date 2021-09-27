#include <windows.h>
#include "resource.h"
BOOL AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HINSTANCE hInstance;
    switch (message)
    {
    case WM_CREATE:
        hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_APP_ABOUT:
            DialogBoxW(hInstance, MAKEINTRESOURCEW(ABOUTBOX), hwnd, (DLGPROC)AboutDlgProc);
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
LRESULT EllipPushWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR       szText[40];
    HBRUSH      hBrush;
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rect;

    switch (message)
    {
    case WM_PAINT:
        GetClientRect(hwnd, &rect);
        GetWindowTextW(hwnd, szText, sizeof(szText));
        hdc = BeginPaint(hwnd, &ps);
        hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        hBrush = (HBRUSH)SelectObject(hdc, hBrush);
        SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
        DrawTextW(hdc, szText, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        DeleteObject(SelectObject(hdc, hBrush));
        EndPaint(hwnd, &ps);
        return 0;
    case WM_KEYUP:
        if (wParam != VK_SPACE)
            break;
    case WM_LBUTTONUP:
        SendMessageW(GetParent(hwnd), WM_COMMAND, GetWindowLongPtrW(hwnd, GWLP_ID), (LPARAM)hwnd);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR szCmdLine, _In_ int iCmdShow)
{
    static TCHAR szAppName[] = L"About3";
    MSG msg;
    HWND hwnd;
    WNDCLASSW wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCEW(ABOUT_ICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = MAKEINTRESOURCEW(ABOUT3);
    wndclass.lpszClassName = szAppName;
    if (!RegisterClassW(&wndclass))
    {
        MessageBoxW(NULL, L"This program requires Windows NT!", szAppName, MB_ICONERROR);
        return 0;
    }
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = EllipPushWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = L"EllipPush";
    RegisterClass(&wndclass);
    hwnd = CreateWindowW(szAppName, L"About Box Demo Program", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}