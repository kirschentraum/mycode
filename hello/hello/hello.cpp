#include <stdio.h>  
#include <math.h>  
#include <windows.h>  
#include <cstdlib>

int main(int argc, char* argv[])
{
	// 物理尺寸
	int nScreenPhysicsWidth = 0, nScreenPhysicsHeight = 0;
	// 像素尺寸
	long nScreenPixelWidth = 0, nScreenPixelHeight = 0;
	// 屏幕刷新频率
	long nDisplayFrequency = 0;

	// 获取物理尺寸
	HDC hdcScreen = ::GetDC(NULL);
	nScreenPhysicsWidth = ::GetDeviceCaps(hdcScreen, HORZSIZE);
	nScreenPhysicsHeight = ::GetDeviceCaps(hdcScreen, VERTSIZE);
	::ReleaseDC(NULL, hdcScreen);

	// 获取像素尺寸和刷新频率
	DEVMODE   dm;
	dm.dmSize = sizeof(DEVMODE);
	::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	nScreenPixelWidth = dm.dmPelsWidth;
	nScreenPixelHeight = dm.dmPelsHeight;
	nDisplayFrequency = dm.dmDisplayFrequency;

	printf("屏幕物理尺寸 : 宽: %d mm, 高: %dmm.\n", nScreenPhysicsWidth, nScreenPhysicsHeight);
	printf("屏幕分辨率 : 宽: %d px, 高: %d px.\n", nScreenPixelWidth, nScreenPixelHeight);
	printf("屏幕刷新频率 : %d Hz.\n", nDisplayFrequency);

	// 1in(英寸) = 25.4mm(毫米)
	const double MILLIMETRE_2_INCH = 1 / 25.4;
	// 计算对角线长度
	double diagonalLen = sqrt(nScreenPhysicsWidth * nScreenPhysicsWidth + nScreenPhysicsHeight * nScreenPhysicsHeight);
	printf("屏幕对角线长为：%.2lf mm, 约 %.2lf in.\n", diagonalLen, diagonalLen * MILLIMETRE_2_INCH);

	return 0;
}