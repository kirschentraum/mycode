#include <windows.h>
#include <math.h>
#include <process.h>
#include <stdio.h>

constexpr long long REP = 10000000000; //Translator: the original value 1000000 is too small, increase 10 times to be 10000000;

#define STATUS_READY     0
#define STATUS_WORKING   1
#define STATUS_DONE      2

#define WM_CALC_DONE     (WM_USER + 0)
#define WM_CALC_ABORTED  (WM_USER + 1)

typedef struct
{
    HWND hwnd;
    BOOL bContinue;
}
PARAMS, * PPARAMS;

LRESULT APIENTRY WndProc(HWND, UINT, WPARAM, LPARAM);

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR szCmdLine, _In_ int nCmdShow)
{
    static TCHAR szAppName[] = TEXT("BigJob1");
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

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, TEXT("Multithreading Demo"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void Thread(PVOID pvoid)
{
    double           A = 1.0;
    long long        i;
    long long        lTime;
    volatile PPARAMS pparams;

    pparams = (PPARAMS)pvoid;

    lTime = GetTickCount64();

    for (i = 0; i < REP && pparams->bContinue; i++)
        A = tan(atan(exp(log(sqrt(A * A))))) + 1.0;

    if (i == REP)
    {
        lTime = GetTickCount64() - lTime;
        SendMessage(pparams->hwnd, WM_CALC_DONE, 0, lTime);
    }
    else
        SendMessage(pparams->hwnd, WM_CALC_ABORTED, 0, 0);

    _endthread();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static INT  iStatus;
    static LONG    lTime;
    static PARAMS  params;
    static const TCHAR* szMessage[] = { TEXT("Ready (left mouse button begins)"),
                                   TEXT("Working (right mouse button ends)"),
                                   TEXT("%lld repetitions in %ld msec") };
    HDC            hdc;
    PAINTSTRUCT    ps;
    RECT           rect;
    TCHAR          szBuffer[64];

    switch (message)
    {
    case WM_LBUTTONDOWN:
        if (iStatus == STATUS_WORKING)
        {
            MessageBeep(0);
            return 0;
        }

        iStatus = STATUS_WORKING;

        params.hwnd = hwnd;
        params.bContinue = TRUE;

        _beginthread(Thread, 0, &params);

        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_RBUTTONDOWN:
        params.bContinue = FALSE;
        return 0;

    case WM_CALC_DONE:
        lTime = lParam;
        iStatus = STATUS_DONE;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_CALC_ABORTED:
        iStatus = STATUS_READY;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        GetClientRect(hwnd, &rect);

        swprintf(szBuffer, 64, szMessage[iStatus], REP, lTime);
        DrawText(hdc, szBuffer, -1, &rect,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
