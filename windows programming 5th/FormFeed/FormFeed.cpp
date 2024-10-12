#include <windows.h>
HDC GetPrinterDC(void)
{
    DWORD            dwNeeded, dwReturned;
    HDC              hdc;
    PRINTER_INFO_4* pinfo4;
    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL, 0, &dwNeeded, &dwReturned);
    pinfo4 = (PRINTER_INFO_4*)malloc(dwNeeded);
    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, (PBYTE)pinfo4, dwNeeded, &dwNeeded, &dwReturned);
    hdc = CreateDC(NULL, pinfo4->pPrinterName, NULL, NULL);
    free(pinfo4);
    return hdc;
}

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int iCmdShow)
{
    static DOCINFO di = { sizeof(DOCINFO), TEXT("FormFeed") };
    HDC            hdcPrint = GetPrinterDC();
    if (hdcPrint != NULL)
    {
        if (StartDoc(hdcPrint, &di) > 0)
            if (StartPage(hdcPrint) > 0 && EndPage(hdcPrint) > 0)
                EndDoc(hdcPrint);
        DeleteDC(hdcPrint);
    }
    return 0;
}