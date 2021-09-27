#include<Windows.h>
#define NUMLINES ((int)(sizeof devcaps/sizeof devcaps[0]))
struct
{
	int iIndex;
	const wchar_t* szLabel;
	const wchar_t* szDesc;
}devcaps[] =
{
HORZSIZE, L"HORZSIZE", L"Width in millimeters",
VERTSIZE, L"VERTSIZE", L"Height in millimeters",
HORZRES, L"HORZRES", L"Width in pixels",
VERTRES, L"VERTRES", L"Height in raster lines",
BITSPIXEL, L"BITSPIXEL", L"Color bits per pixel",
PLANES, L"PLANES", L"Number of Color Planes",
NUMBRUSHES, L"NUMBRUSHES", L"Number of device brushes",
NUMPENS, L"NUMPENS", L"Number of device pens",
NUMMARKERS, L"NUMMARKERS", L"Number of device markers",
NUMFONTS, L"NUMFONTS", L"Number of device fonts",
NUMCOLORS, L"NUMCOLORS", L"Number of device colors",
PDEVICESIZE, L"PDEVICESIZE", L"Size of device structure",
ASPECTX, L"ASPECTX", L"Relative width of pixel",
ASPECTY, L"ASPECTY", L"Relative height of pixel",
ASPECTXY, L"ASPECTXY", L"Relative diagonal of pixel",
LOGPIXELSX, L"LOGPIXELSX", L"Horizontal dots per inch",
LOGPIXELSY, L"LOGPIXELSY", L"Vertical dots per inch",
SIZEPALETTE, L"SIZEPALETTE", L"Number of palette entries",
NUMRESERVED, L"NUMRESERVED", L"Reserved palette entries",
COLORRES, L"COLORRES", L"Actual color resolution"
};
LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	static int cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth;
	HDC hdc;
	PAINTSTRUCT ps;
	int i, x, y, iVertPos, iHorzPos, iPaintBeg, iPaintEnd;
	wchar_t buffer[10];
	TEXTMETRICW tm;
	SCROLLINFO si;
	switch (message)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		GetTextMetricsW(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);
		iMaxWidth = 40 * cxChar + 22 * cxCaps;
		return 0;
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = NUMLINES - 1;
		si.nPage = cyClient / cyChar;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 2 + iMaxWidth / cxChar;
		si.nPage = cxClient / cxChar;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		return 0;
	case WM_VSCROLL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_THUMBPOSITION:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);
		if (si.nPos != iVertPos)
		{
			ScrollWindow(hwnd, 0, cyChar * (iVertPos - si.nPos), NULL, NULL);
			//UpdateWindow(hwnd);
		}
		return 0;

	case WM_HSCROLL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_LINELEFT:
			si.nPos -= 1;
			break;
		case SB_LINERIGHT:
			si.nPos += 1;
			break;
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;
		case SB_THUMBPOSITION:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);
		if (si.nPos != iHorzPos)
		{
			ScrollWindow(hwnd, cxChar * (iHorzPos - si.nPos), 0, NULL, NULL);
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		iPaintBeg = max(0, iVertPos + ps.rcPaint.top / cyChar);
		iPaintEnd = min(NUMLINES - 1, iVertPos + ps.rcPaint.bottom / cyChar);
		for (i = iPaintBeg; i <= iPaintEnd; i++)
		{
			x = cxChar * (1 - iHorzPos);
			y = cyChar * (i - iVertPos);
			TextOutW(hdc, x, y, devcaps[i].szLabel, lstrlenW(devcaps[i].szLabel));
			TextOutW(hdc, x + 22 * cxCaps, y, devcaps[i].szDesc, lstrlenW(devcaps[i].szDesc));
			SetTextAlign(hdc, TA_RIGHT | TA_TOP);
			TextOutW(hdc, x + 22 * cxCaps + 40 * cxChar, y, buffer, wsprintfW(buffer, L"%5d", GetDeviceCaps(hdc, devcaps[i].iIndex)));
			SetTextAlign(hdc, TA_LEFT | TA_TOP);
		}
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, message, wParam, lParam);
}
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	wchar_t appName[] = L"DevCaps";
	HWND hwnd;
	MSG msg;
	WNDCLASSW wndclass = {};
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	wndclass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = appName;
	if (!RegisterClassW(&wndclass))
	{
		MessageBoxW(NULL, L"This Program requires Windows NT!", appName, MB_ICONERROR);
		return 0;
	}
	hwnd = CreateWindowExW(0L, appName, L"Get System Metrics No.1", WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, nShowCmd);
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}