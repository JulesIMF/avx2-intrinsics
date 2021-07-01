#pragma once
#ifndef JULESIMF_LOADBMP
#define JULESIMF_LOADBMP

struct BMP
{
    int xSize, ySize;
    int bitCount;
    char* image;
};

BMP loadImage(FILE* file);

#endif
