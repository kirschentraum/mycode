#include <windows.h>
#include <commdlg.h>

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR szCmdLine, _In_ int iCmdShow)
{
    static CHOOSECOLOR cc;
    static COLORREF    crCustColors[16];
    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = NULL;
    cc.hInstance = NULL;
    cc.rgbResult = RGB(0x80, 0x80, 0x80);
    cc.lpCustColors = crCustColors;
    cc.Flags = CC_RGBINIT | CC_FULLOPEN;
    cc.lCustData = 0;
    cc.lpfnHook = NULL;
    cc.lpTemplateName = NULL;
    return ChooseColor(&cc);
}