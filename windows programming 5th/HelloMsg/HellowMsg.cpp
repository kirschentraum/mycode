//本程序显示一个信息窗口
#include <windows.h>
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int nCmdShow)
{
    MessageBoxW(NULL, L"Hello, Windows 11!", L"HelloMsg", 0);
}