/*
Copyright (c) 2021  MIPT

Module Name:
    Source.cpp

Abstract:    
    
Author:
    JulesIMF

Last Edit:
    8.4.2021 21:29

Edit Notes:
    
*/
#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "Intrinsic_2.h"
#include <cstdio>
#include <cmath>
#include <cassert>
#include <immintrin.h>
#include "WinAPI_lib.h"
#include "LoadBMP.h"

#define XSIZE          800                        // размер окна по горизонтали
#define YSIZE          600                        // размер окна по вертикали
static_assert(YSIZE % 4 == 0 && XSIZE % 4 == 0, "XSIZE and YSIZE must be multiples of 4!");

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void                Redraw(HWND, bool toUpdateFps = false);

class Calculate
{
    inline static bool isAvx = true;
    inline static int  loops = 50;
    inline static BMP  table = {},
                       cat = {};

    inline static int  xCatStatic = 128,
                       yCatStatic = 144;

public:
    void static SwitchAvx(void)
    {
        isAvx = !isAvx;
    }

    void static SwitchLoops(void)
    {
        loops = 750 - loops;
    }

    void static SetTable(BMP newValue)
    {
        table = newValue;
    }

    void static SetCat(BMP newValue)
    {
        cat = newValue;
    }

    void static SetXCatStatic(int delta)
    {
        int newX = xCatStatic + delta;


        // 
        // проверяем выравнивание
        //
        if (newX % 8 != 0)
        {
            MessageBox(nullptr, L"Неверный xCat (не кратен 8)!", L"Кот Димы Евтушенко", MB_ICONERROR);
            exit(10);
        }

        if (newX >= 0 && newX + cat.xSize <= XSIZE)
            xCatStatic = newX;

        needReload = true;
    }

    void static SetYCatStatic(int delta)
    {
        int newY = yCatStatic + delta;

        if (newY >= 0 && newY + cat.ySize <= YSIZE)
            yCatStatic = newY;

        needReload = true;
    }

    static void DoRoutine(void);

    inline static HWND hWnd         = {};
    inline static char* tableBuffer = nullptr;
    inline static bool needReload   = false;


    static DWORD WINAPI StartCalculate(LPVOID lpThreadParameter)
    {
        while (true)
        {
            if(needReload)
                memcpy(Videomemory::image, tableBuffer, XSIZE * YSIZE * 4),
                needReload = false;

            DoRoutine();
            Redraw(hWnd);
        }
    }
};

int _stdcall wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    LPCWSTR const className = L"CatTable";

    Application::SetWndProc(WndProc);
    WNDCLASSEXW  wndClass{ sizeof(WNDCLASSEXW) };
    Application::FillStdWndClass(&wndClass, className);
    Application::RegisterWndClass(&wndClass);
    Videomemory::Initialize(XSIZE, YSIZE);

    auto hWnd = Application::MakeWindow(className,
                                        nCmdShow,
                                        L"Кот Димы Евтушенко",
                                        (WS_OVERLAPPEDWINDOW) ^ (WS_THICKFRAME | WS_MAXIMIZE | WS_MAXIMIZEBOX)
    );

    if (!hWnd)
    {
        MessageBox(hWnd,
                   L"Не удалось создать окно (hwnd == 0)",
                   L"Ошибка MakeWindow",
                   MB_ICONERROR);

        return FALSE;
    }

    //
    // Загрузим изображения
    //
    FILE* table = fopen("Table.bmp",   "rb");
    FILE* cat   = fopen("DimaCat.bmp", "rb");
    if (!table)
    {
        MessageBox(nullptr, L"Не удалось открыть Table.bmp", L"Кот", MB_ICONERROR);
        return 1;
    }

    if (!cat)
    {
        MessageBox(nullptr, L"Не удалось открыть DimaCat.bmp", L"Кот", MB_ICONERROR);
        return 2;
    }

    auto bmpTable = loadImage(table),
         bmpCat   = loadImage(cat);

    fclose(table);
    fclose(cat);

    // 
    // проверим целостность изображений
    //
    if (!bmpTable.xSize)
    {
        MessageBox(nullptr, L"Изображение стола повреждено", L"Кот", MB_ICONERROR);
        return 3;
    }

    if (bmpTable.xSize != XSIZE || bmpTable.ySize != YSIZE)
    {
        MessageBox(nullptr, L"Изображение стола имеет неправильные размеры (не 800х600)", L"Кот", MB_ICONERROR);
        return 4;
    }

    if (!bmpCat.xSize)
    {
        MessageBox(nullptr, L"Изображение кота повреждено", L"Кот", MB_ICONERROR);
        return 5;
    }

    memcpy(Videomemory::image, bmpTable.image, XSIZE * YSIZE * 4); // загрузим стол в видеопамять

    Calculate::SetCat(bmpCat);      // передадим в Calculate изображения
    Calculate::SetTable(bmpTable);  //

    Calculate::hWnd = hWnd;
    Calculate::tableBuffer = bmpTable.image;

    //
    // запустим кота считаться
    //
    auto calculateThread = CreateThread(
        nullptr,
        0,
        Calculate::StartCalculate,
        nullptr,
        0,
        0);

    if (!calculateThread)
    {
        MessageBox(hWnd,
                   L"Не удалось запустить поток рассчета изображения (calculateThread == nullptr)",
                   L"Ошибка CreateThread",
                   MB_ICONERROR);

        return FALSE;
    }


    //
    // здесь будем обрабатывать нажатия клавиш и другие сообщения
    //
    Application::ProceedMessages();

    TerminateThread(calculateThread, 0);
    Videomemory::Delete();
    free(bmpCat.image);
    free(bmpTable.image);

    return 0;
}


//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // 
    // Эти переменные отвечают за расположение картинки
    // Они хранятся здесь, т.к. в этой функции обработка клавиш
    static float xCenter = -1.325f,
        yCenter = 0.0,
        step = (1.f / 800.f);

    static bool  isFast = false;
    static bool  isSingle = true;

    float const
        shift = 30.0,
        deltaScale = 1.33f,
        xMin = -1.42,
        xMax = 0.33,
        yMin = -1.43,
        yMax = 1.43,
        stepMax = 0.00691;

    switch (message)
    {
        /*
        *   УПРАВЛЕНИЕ КЛАВИШАМИ
        *
        *       UP      - поднять вверх
        *       DOWN    - опустить вниз
        *       LEFT    - передвинуть налево
        *       RIGHT   - передвинуть нараво
        *       PLUS    - приблизиться
        *       MINUS   - отдалиться
        */
        HANDLE_MESSAGE(WM_KEYDOWN,
                       {
                           switch (wParam)
                           {
                               HANDLE_MESSAGE(VK_UP,
                                              {
                                                   Calculate::SetYCatStatic(8);
                                              })

                               HANDLE_MESSAGE(VK_DOWN,
                                              {
                                                   Calculate::SetYCatStatic(-8);
                                              })

                               HANDLE_MESSAGE(VK_LEFT,
                                              {
                                                   Calculate::SetXCatStatic(-8);
                                              })

                               HANDLE_MESSAGE(VK_RIGHT,
                                              {
                                                   Calculate::SetXCatStatic(8);
                                              })

                               HANDLE_MESSAGE(VK_TAB,
                                              {
                                                   Calculate::SwitchAvx();
                                              })

                               HANDLE_MESSAGE(VK_SPACE,
                                              {
                                                   Redraw(hWnd, true); // хотим переключить заморозку
                                              })

                           default:
                               break;
                           }
                       })

            HANDLE_MESSAGE(WM_PAINT,
                           {
                                Redraw({ hWnd });
                           })

                           HANDLE_MESSAGE(WM_DESTROY,
                                          {
                                               MessageBox(hWnd, L"Fine release by JulesIMF!", L"Кот Димы Евтушенко", MB_ICONINFORMATION);
                                               PostQuitMessage(0);
                                          })

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/// <summary>
/// Перерисовывает буфер в окно и выводит FPS
/// </summary>
/// <param name="hWnd">хендлер окна</param>
/// <param name="switchUpdateFps">если true, то переключается режим заморозки</param>
void Redraw(HWND hWnd, bool switchUpdateFps)
{
    static DWORD previousTime = 0;
    static float fps = 0;
    static bool  toUpdateFps = true;

    if (switchUpdateFps)
    {
        toUpdateFps = !toUpdateFps;
        return;
    }

    HDC hdc = Videomemory::Draw(hWnd);

    /*
    *   ВЫВОД FPS
    */
    DWORD currentTime = GetTickCount();

    int     nOut = 0;
    wchar_t bufferForFps[32];
    char    bufferForPrintf[32];

    if (toUpdateFps) // обновляем только если стоит соотв. флаг
        fps = 1000.f / (currentTime - previousTime);

    snprintf(bufferForPrintf, 31, "%.2f", fps);
    nOut = wsprintf(bufferForFps, L"%S", bufferForPrintf);
    assert(nOut);
    previousTime = currentTime;
    TextOut(hdc, 0, 0, bufferForFps, nOut);

    EndPaint(hWnd, nullptr);
}

//
// Раз в Интеле не сделали такого интринсика, 
// то придется брать ситуацию в свои руки
//
inline __m256i _mm256_mulhi_epu8(__m256i a, __m256i b)
{
    __m256i aCopy = a, bCopy = b;

    __m256i maskVast        = _mm256_set_epi8(-1, 15, -1, 14, -1, 13, -1, 12,
                                              -1, 11, -1, 10, -1, 9,  -1, 8,
                                              -1, 7,  -1, 6,  -1, 5,  -1, 4,
                                              -1, 3,  -1, 2,  -1, 1,  -1, 0);

    __m256i maskConsolidate = _mm256_set_epi8(15, 13, 11,  9,  7,  5,  3,  1,
                                              -1, -1, -1, -1, -1, -1, -1, -1,
                                              -1, -1, -1, -1, -1, -1, -1, -1,
                                              15, 13, 11,  9,  7,  5,  3,  1);


    __m256i aLowDup  = _mm256_broadcastsi128_si256(_mm256_castsi256_si128(a));
    __m256i bLowDup  = _mm256_broadcastsi128_si256(_mm256_castsi256_si128(b));

    __m256i aHighDup = a; *((__m128i*)(&aHighDup)) = *((__m128i*)(&aHighDup) + 1); // почему нельзя красиво запихать верхнюю часть ymm в xmm?((
    __m256i bHighDup = b; *((__m128i*)(&bHighDup)) = *((__m128i*)(&bHighDup) + 1); //



    __m256i aLowVast  = _mm256_shuffle_epi8(aLowDup,  maskVast); // 
    __m256i bLowVast  = _mm256_shuffle_epi8(bLowDup,  maskVast); // расставляем широко числа
    __m256i aHighVast = _mm256_shuffle_epi8(aHighDup, maskVast); // 
    __m256i bHighVast = _mm256_shuffle_epi8(bHighDup, maskVast); //

    aLowVast  = _mm256_mullo_epi16(aLowVast,  bLowVast);  // перемножаем
    aHighVast = _mm256_mullo_epi16(aHighVast, bHighVast); //

    aLowVast  = _mm256_shuffle_epi8(aLowVast,  maskConsolidate); // консолидируем, то есть собираем вместе старшие разряды произведений
    aHighVast = _mm256_shuffle_epi8(aHighVast, maskConsolidate); //
    
    *(__m128i*)(&aLowVast)        = _mm_or_si128(_mm256_castsi256_si128(aLowVast),  *((__m128i*)(&aLowVast)  + 1)); // объединяем старший ymm и xmm
    *((__m128i*)(&aHighVast) + 1) = _mm_or_si128(_mm256_castsi256_si128(aHighVast), *((__m128i*)(&aHighVast) + 1)); //
    *(__m128i*)(&aHighVast)       = _mm_setzero_si128(); // зануляем то что уже не надо
    *((__m128i*)(&aLowVast) + 1)  = _mm_setzero_si128(); //
    

    auto ans = _mm256_or_si256(aHighVast, aLowVast); // объединяем то что получилось

    return ans;
}

void Calculate::DoRoutine(void)
{
    int xCat = xCatStatic, yCat = yCatStatic;
    if (isAvx)
    {
        for (int nCalc = 0; nCalc != loops; nCalc++)
        {
            __m256i _m1      = _mm256_set1_epi32(-1);
            __m256i alMask   = _mm256_set_epi8(15, 15, 15, 15,   // маска для размножения альфа-канала
                                               11, 11, 11, 11,
                                               7,  7,  7,  7,
                                               3,  3,  3,  3,
                                               15, 15, 15, 15,
                                               11, 11, 11, 11,
                                               7,  7,  7,  7,
                                               3,  3,  3,  3);
            int catIndex = 0;

            for (int j = 0; j != cat.ySize; j++) // уважаем кэш
            {
                int index = (XSIZE * (j + yCat) + xCat);

                int i = 0;                                                      // ширина кота может быть не кратна 8, 
                for (; i + 7 <= cat.xSize; i += 8, index += 8, catIndex += 8)   // поэтому если что, надо будет дообработать снизу
                {
                    __m256i rgbTable  = *(__m256i*)(table.image + index    * sizeof(RGBQUAD)),
                            rgbCat    = *(__m256i*)(cat.image   + catIndex * sizeof(RGBQUAD));


                    __m256i alphaCat   = _mm256_shuffle_epi8(rgbCat, alMask);       // получаем пачки по 4 альфа-канала для кота
                    __m256i alphaTable = _mm256_sub_epi8(_m1, alphaCat);            // для стола

                    __m256i blendCat   = _mm256_mulhi_epu8(alphaCat,   rgbCat);     // это замешанный кот
                    __m256i blendTable = _mm256_mulhi_epu8(alphaTable, rgbTable);   // а это замешанный стол

                    auto res = _mm256_add_epi8(blendCat, blendTable);               // в результате просто надо сложить
                    *(__m256i*)((RGBQUAD*)Videomemory::image + index) = res;
                }
                                                                                    // а это обещанная дообработка того, 
                for (; i != cat.xSize; i++, index++)                                // не поместилось в кратные 8 куски
                {
                    RGBQUAD rgbTable = *((RGBQUAD*)table.image + index),
                            rgbCat   = *((RGBQUAD*)cat.image   + catIndex++);

                    RGBQUAD result =
                    {
                        (rgbTable.rgbBlue  * (255 - rgbCat.rgbReserved) + rgbCat.rgbBlue  * rgbCat.rgbReserved) >> 8,
                        (rgbTable.rgbGreen * (255 - rgbCat.rgbReserved) + rgbCat.rgbGreen * rgbCat.rgbReserved) >> 8,
                        (rgbTable.rgbRed   * (255 - rgbCat.rgbReserved) + rgbCat.rgbRed   * rgbCat.rgbReserved) >> 8,
                        255
                    };

                    *((RGBQUAD*)Videomemory::image + index) = result;
                }

            }
        }
    }

    else
    {
    notAvx:
        for (int nCalc = 0; nCalc != loops; nCalc++)
        {
            int catIndex = 0;
            for (int j = 0; j != cat.ySize; j++)
            {
                int index = (XSIZE * (j + yCat) + xCat);
                for (int i = 0; i != cat.xSize; i++, index++)
                {
                    RGBQUAD rgbTable = *((RGBQUAD*)table.image + index),
                            rgbCat   = *((RGBQUAD*)cat.image   + catIndex++);

                    RGBQUAD result   = 
                    {
                        ( rgbTable.rgbBlue  * (255 - rgbCat.rgbReserved) + rgbCat.rgbBlue  * rgbCat.rgbReserved ) >> 8,
                        ( rgbTable.rgbGreen * (255 - rgbCat.rgbReserved) + rgbCat.rgbGreen * rgbCat.rgbReserved ) >> 8,
                        ( rgbTable.rgbRed   * (255 - rgbCat.rgbReserved) + rgbCat.rgbRed   * rgbCat.rgbReserved ) >> 8,
                        255
                    };

                    *((RGBQUAD*)Videomemory::image + index) = result;
                }
            }
        }
    }
}
