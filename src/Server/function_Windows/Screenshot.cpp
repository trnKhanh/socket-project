#include "Screenshot.h"

#include <gdiplus.h>
#include <fstream>

using namespace Gdiplus;

int screenshotHelper(std::vector<char> &buffer){
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    DEVMODE dm;
    dm.dmSize = sizeof(dm);

    int screenWidth, screenHeight;

    if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm)){
        screenWidth = dm.dmPelsWidth;
        screenHeight = dm.dmPelsHeight;
    }
    else
        return -1;

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);

    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    CLSID clsid;
    int status = GetEncoderClsid(L"image/png", &clsid);
    if(status == -1){

        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);

        // GdiplusShutdown(gdiplusToken);
        return -1;
    }

    Bitmap bmp(hBitmap, (HPALETTE)0);
    bmp.Save(L"screenshot.png", &clsid, NULL);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // GdiplusShutdown(gdiplusToken);

    std::ifstream fi("screenshot.png", std::ios::binary);
    if(fi.fail())
        return -1;
    buffer = std::vector<char>(std::istreambuf_iterator<char>(fi), {});

    fi.close();
    status = remove("screenshot.png");

    return 0;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
    UINT  num = 0;  // number of image encoders
    UINT  size = 0;  // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j){
        if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0){
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}