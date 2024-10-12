#include <windows.h>
constexpr auto ID_LIST = 1;
constexpr auto ID_TEXT = 2;
LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
void FillListBox(HWND hwndList)
{
    long long iLength;
    TCHAR* pVarBlock, * pVarBeg, * pVarEnd, * pVarName, *pVarBlockBegin;
    pVarBlockBegin = GetEnvironmentStringsW();
    pVarBlock = pVarBlockBegin;
    while (*pVarBlock)
    {
        if (*pVarBlock != '=')
        {
            pVarBeg = pVarBlock;
            while (*pVarBlock++ != '=');
            pVarEnd = pVarBlock - 1;
            iLength = pVarEnd - pVarBeg;
            pVarName = (TCHAR*)calloc((long long)iLength + 1, sizeof(TCHAR));
            if (pVarName == NULL)
                break;
            CopyMemory(pVarName, pVarBeg, iLength * sizeof(TCHAR));
            pVarName[iLength] = '\0';
            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)pVarName);
            free(pVarName);
        }
        while (*pVarBlock++ != '\0');     // Scan until terminating zero
    }
    FreeEnvironmentStringsW(pVarBlockBegin);
}
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR szCmdLine, _In_ int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("Environ");
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
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }
    hwnd = CreateWindow(szAppName, TEXT("Environment List Box"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
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
    static HWND  hwndList, hwndText;
    int          iIndex, iLength, cxChar, cyChar;
    TCHAR* pVarName, * pVarValue;

    switch (message)
    {
    case WM_CREATE:
        cxChar = LOWORD(GetDialogBaseUnits());
        cyChar = HIWORD(GetDialogBaseUnits());
        hwndList = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD, cxChar, cyChar * 3, cxChar * 16 + GetSystemMetrics(SM_CXVSCROLL), cyChar * 5, hwnd, (HMENU)(long long)ID_LIST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        hwndText = CreateWindow(TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_LEFT, cxChar, cyChar, GetSystemMetrics(SM_CXSCREEN), cyChar, hwnd, (HMENU)(long long)ID_TEXT,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        FillListBox(hwndList);
        return 0;
    case WM_SETFOCUS:
        SetFocus(hwndList);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_SELCHANGE)
        {
            iIndex = (int)SendMessageW(hwndList, LB_GETCURSEL, 0, 0);
            iLength = (int)SendMessageW(hwndList, LB_GETTEXTLEN, iIndex, 0) + 1;
            pVarName = (TCHAR*)calloc(iLength, sizeof(TCHAR));
            SendMessageW(hwndList, LB_GETTEXT, iIndex, (LPARAM)pVarName);
            iLength = GetEnvironmentVariable(pVarName, NULL, 0);
            pVarValue = (TCHAR*)calloc(iLength, sizeof(TCHAR));
            GetEnvironmentVariable(pVarName, pVarValue, iLength);
            SetWindowText(hwndText, pVarValue);
            free(pVarName);
            free(pVarValue);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}