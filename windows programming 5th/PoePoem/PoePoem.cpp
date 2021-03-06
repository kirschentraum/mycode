#include <windows.h>
#include "resource.h"

LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE hInst;

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR szCmdLine, _In_ int iCmdShow)
{
    TCHAR    szAppName[16], szCaption[64], szErrMsg[64];
    HWND     hwnd;
    MSG      msg;
    WNDCLASS wndclass;

    LoadString(hInstance, IDS_APPNAME, szAppName,
        sizeof(szAppName) / sizeof(TCHAR));

    LoadString(hInstance, IDS_CAPTION, szCaption,
        sizeof(szCaption) / sizeof(TCHAR));

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, szAppName);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        LoadStringA(hInstance, IDS_APPNAME, (char*)szAppName,
            sizeof(szAppName));

        LoadStringA(hInstance, IDS_ERRMSG, (char*)szErrMsg,
            sizeof(szErrMsg));

        MessageBoxA(NULL, (char*)szErrMsg,
            (char*)szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, szCaption,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

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
    static char* pText, *pTextBegin, *pTextLock;
    static HGLOBAL hResource;
    static HWND    hScroll;
    static int     iPosition, cxChar, cyChar, cyClient, iNumLines, xScroll;
    HDC            hdc;
    PAINTSTRUCT    ps;
    RECT           rect;
    TEXTMETRIC     tm;
    HRSRC hrsrc;

    switch (message)
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);
        GetTextMetrics(hdc, &tm);
        cxChar = tm.tmAveCharWidth;
        cyChar = tm.tmHeight + tm.tmExternalLeading;
        ReleaseDC(hwnd, hdc);

        xScroll = GetSystemMetrics(SM_CXVSCROLL);

        hScroll = CreateWindow(TEXT("scrollbar"), NULL,
            WS_CHILD | WS_VISIBLE | SBS_VERT,
            0, 0, 0, 0,
            hwnd, (HMENU)1, hInst, NULL);
        hrsrc = FindResource(hInst, (LPCWSTR)ANNABELLEE, TEXT("TEXT"));
        hResource = LoadResource(hInst, hrsrc);

        pTextLock = (char*)LockResource(hResource);
        pTextBegin = pTextLock;
        iNumLines = 0;

        while (*pTextLock != '\\' && *pTextLock != '\0')
        {
            if (*pTextLock == '\n')
                iNumLines++;
            pTextLock = AnsiNext(pTextLock);
        }
        pText = new char[pTextLock - pTextBegin + 1];
        memcpy(pText, pTextBegin, pTextLock - pTextBegin);
        pText[pTextLock - pTextBegin] = '\0';

        SetScrollRange(hScroll, SB_CTL, 0, iNumLines, FALSE);
        SetScrollPos(hScroll, SB_CTL, 0, FALSE);
        return 0;

    case WM_SIZE:
        MoveWindow(hScroll, LOWORD(lParam) - xScroll, 0,
            xScroll, cyClient = HIWORD(lParam), TRUE);
        SetFocus(hwnd);
        return 0;

    case WM_SETFOCUS:
        SetFocus(hScroll);
        return 0;

    case WM_VSCROLL:
        switch (wParam)
        {
        case SB_TOP:
            iPosition = 0;
            break;
        case SB_BOTTOM:
            iPosition = iNumLines;
            break;
        case SB_LINEUP:
            iPosition -= 1;
            break;
        case SB_LINEDOWN:
            iPosition += 1;
            break;
        case SB_PAGEUP:
            iPosition -= cyClient / cyChar;
            break;
        case SB_PAGEDOWN:
            iPosition += cyClient / cyChar;
            break;
        case SB_THUMBPOSITION:
            iPosition = LOWORD(lParam);
            break;
        }
        iPosition = max(0, min(iPosition, iNumLines));

        if (iPosition != GetScrollPos(hScroll, SB_CTL))
        {
            SetScrollPos(hScroll, SB_CTL, iPosition, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);



        GetClientRect(hwnd, &rect);
        rect.left += cxChar;
        rect.top += cyChar * (1 - iPosition);
        DrawTextA(hdc, pText, -1, &rect, DT_EXTERNALLEADING);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        FreeResource(hResource);
        if (pText != NULL)
            delete[] pText;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
