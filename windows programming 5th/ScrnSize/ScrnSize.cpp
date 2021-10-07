//本程序显示屏幕真实分辨率
#include <windows.h>
#include <tchar.h>     
#include <stdio.h>
int MessageBoxPrintf(const wchar_t* szCaption, const wchar_t* szFormat, ...)
{
    wchar_t szBuffer[1024];
    va_list pArgList;
    va_start(pArgList, szFormat);
    _vsnwprintf_s(szBuffer, sizeof(szBuffer) / sizeof(wchar_t), szFormat, pArgList);
    va_end(pArgList);
    return MessageBoxW(NULL, szBuffer, szCaption, 0);
}
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    int cxScreen, cyScreen;
    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);
    MessageBoxPrintf(L"ScrnSize", L"The screen is %i pixels wide by %i pixels high.", cxScreen, cyScreen);
    return 0;
}