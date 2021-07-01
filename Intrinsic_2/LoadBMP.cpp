#include "framework.h"
#include <cstdio>

#include "LoadBMP.h"

BMP loadImage(FILE* file)
{
    BMP bmp = {};

    BITMAPFILEHEADER fileHeader = {};
    BITMAPINFOHEADER infoHeader = {};

    fseek(file, 0, SEEK_SET);
    if (!fread(&fileHeader, sizeof(fileHeader), 1, file))
        return bmp;

    if (fileHeader.bfType != 'BM' && fileHeader.bfType != 'MB')
        return bmp;

    if (!fread(&infoHeader, sizeof(infoHeader), 1, file))
        return bmp;

    bmp.bitCount = infoHeader.biBitCount;
    bmp.image    = (char*) calloc(infoHeader.biWidth * infoHeader.biHeight, bmp.bitCount / 8);
    if (!bmp.image)
        return bmp;

    fseek(file, fileHeader.bfOffBits, SEEK_SET);
    if (fread(bmp.image,
              bmp.bitCount / 8,
              infoHeader.biWidth * infoHeader.biHeight,
              file) 
        != infoHeader.biWidth * infoHeader.biHeight)
    {
        return bmp;
    }

    bmp.xSize    = infoHeader.biWidth;
    bmp.ySize    = infoHeader.biHeight;

    return bmp;
}

