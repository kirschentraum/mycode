#include <windows.h>
int idFocus;
WNDPROC OldScroll[3];
LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT ScrollProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int id = GetWindowLong(hwnd, GWL_ID);
    switch (message)
    {
    case WM_KEYDOWN:
        if (wParam == VK_TAB)
            SetFocus(GetDlgItem(GetParent(hwnd),
                (id + (GetKeyState(VK_SHIFT) < 0 ? 2 : 1)) % 3));
        break;
    case WM_SETFOCUS:
        idFocus = id;
        break;
    }
    return CallWindowProc(OldScroll[id], hwnd, message, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR szCmdLine, _In_ int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("Colors1");
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
    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }
    hwnd = CreateWindow(szAppName, TEXT("Color Scroll"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
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
    static COLORREF crPrim[3] = { RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255) };
    static HBRUSH  hBrush[3], hBrushStatic;
    static HWND    hwndScroll[3], hwndLabel[3], hwndValue[3], hwndRect;
    static int     color[3], cyChar;
    static RECT    rcColor;
    static const TCHAR* szColorLabel[] = { TEXT("Red"), TEXT("Green"), TEXT("Blue") };
    HINSTANCE      hInstance;
    int            i, cxClient, cyClient;
    TCHAR          szBuffer[10];
    switch (message)
    {
    case WM_CREATE:
        hInstance = (HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE);
        hwndRect = CreateWindow(TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_WHITERECT, 0, 0, 0, 0, hwnd, (HMENU)9, hInstance, NULL);
        for (i = 0; i < 3; i++)
        {
            hwndScroll[i] = CreateWindow(TEXT("scrollbar"), NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | SBS_VERT, 0, 0, 0, 0, hwnd, (HMENU)i, hInstance, NULL);
            SetScrollRange(hwndScroll[i], SB_CTL, 0, 255, FALSE);
            SetScrollPos(hwndScroll[i], SB_CTL, 0, FALSE);
            hwndLabel[i] = CreateWindow(TEXT("static"), szColorLabel[i],
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                0, 0, 0, 0,
                hwnd, (HMENU)((long long)i + (long long)3),
                hInstance, NULL);

            // The three color-value text fields have IDs 6, 7, 
            // and 8, and initial text strings of "0".

            hwndValue[i] = CreateWindow(TEXT("static"), TEXT("0"),
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                0, 0, 0, 0,
                hwnd, (HMENU)((long long)i + (long long)6),
                hInstance, NULL);

            OldScroll[i] = (WNDPROC)SetWindowLong(hwndScroll[i],
                GWLP_WNDPROC, (LONG)ScrollProc);

            hBrush[i] = CreateSolidBrush(crPrim[i]);
        }

        hBrushStatic = CreateSolidBrush(
            GetSysColor(COLOR_BTNHIGHLIGHT));

        cyChar = HIWORD(GetDialogBaseUnits());
        return 0;

    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        SetRect(&rcColor, cxClient / 2, 0, cxClient, cyClient);

        MoveWindow(hwndRect, 0, 0, cxClient / 2, cyClient, TRUE);

        for (i = 0; i < 3; i++)
        {
            MoveWindow(hwndScroll[i],
                (2 * i + 1) * cxClient / 14, 2 * cyChar,
                cxClient / 14, cyClient - 4 * cyChar, TRUE);

            MoveWindow(hwndLabel[i],
                (4 * i + 1) * cxClient / 28, cyChar / 2,
                cxClient / 7, cyChar, TRUE);

            MoveWindow(hwndValue[i],
                (4 * i + 1) * cxClient / 28,
                cyClient - 3 * cyChar / 2,
                cxClient / 7, cyChar, TRUE);
        }
        SetFocus(hwnd);
        return 0;

    case WM_SETFOCUS:
        SetFocus(hwndScroll[idFocus]);
        return 0;

    case WM_VSCROLL:
        i = GetWindowLong((HWND)lParam, GWL_ID);

        switch (LOWORD(wParam))
        {
        case SB_PAGEDOWN:
            color[i] += 15;
            // fall through
        case SB_LINEDOWN:
            color[i] = min(255, color[i] + 1);
            break;

        case SB_PAGEUP:
            color[i] -= 15;
            // fall through
        case SB_LINEUP:
            color[i] = max(0, color[i] - 1);
            break;

        case SB_TOP:
            color[i] = 0;
            break;

        case SB_BOTTOM:
            color[i] = 255;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            color[i] = HIWORD(wParam);
            break;

        default:
            break;
        }
        SetScrollPos(hwndScroll[i], SB_CTL, color[i], TRUE);
        wsprintf(szBuffer, TEXT("%i"), color[i]);
        SetWindowText(hwndValue[i], szBuffer);

        DeleteObject((HBRUSH)SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(color[0], color[1], color[2]))));

        InvalidateRect(hwnd, &rcColor, TRUE);
        return 0;

    case WM_CTLCOLORSCROLLBAR:
        i = GetWindowLong((HWND)lParam, GWL_ID);
        return (LRESULT)hBrush[i];

    case WM_CTLCOLORSTATIC:
        i = GetWindowLong((HWND)lParam, GWL_ID);

        if (i >= 3 && i <= 8)    // static text controls
        {
            SetTextColor((HDC)wParam, crPrim[i % 3]);
            SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNHIGHLIGHT));
            return (LRESULT)hBrushStatic;
        }
        break;

    case WM_SYSCOLORCHANGE:
        DeleteObject(hBrushStatic);
        hBrushStatic = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
        return 0;

    case WM_DESTROY:
        DeleteObject((HBRUSH)
            SetClassLong(hwnd, GCLP_HBRBACKGROUND, (long long)GetStockObject(WHITE_BRUSH)));

        for (i = 0; i < 3; i++)
            DeleteObject(hBrush[i]);

        DeleteObject(hBrushStatic);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

