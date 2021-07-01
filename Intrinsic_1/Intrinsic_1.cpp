/*
Copyright (c) 2021  MIPT

Module Name:
    Intrinsic_1.cpp

Abstract:

Author:
    JulesIMF

Last Edit:
    5.4.2021 12:39

Edit Notes:
    
*/

#include "framework.h"
#include "Intrinsic_1.h"
#include <cstdio>
#include <cmath>
#include <cassert>
#include <immintrin.h>
#include "WinAPI_lib.h"

#define MAX_LOADSTRING 100
#define XSIZE          800                        // размер окна по горизонтали
#define YSIZE          600                        // размер окна по вертикали


static_assert(YSIZE % 4 == 0 && XSIZE % 4 == 0, "XSIZE and YSIZE must be multiples of 4!");

LRESULT _stdcall WndProc(HWND, UINT, WPARAM, LPARAM);

void Redraw(HWND, bool toUpdateFps = false);

class Calculate
{
    inline static float     xCenter = -1.325f, yCenter  = 0.0, step = (1.f / 800.f);
    inline static bool      isFast  = false,   isSingle = true;

    static int   const             maxTransforms = 256;
    inline static float const      maxSqRadius   = 125.f;
    static int   const             scrX = XSIZE, scrY = YSIZE;

public:
    void static SetXCenter(float newValue)
    {
        xCenter = newValue;
    }

    void static SetYCenter(float newValue)
    {
        yCenter = newValue;
    }

    void static SetStep(float newValue)
    {
        step = newValue;
    }

    void static SetIsFast(bool newValue)
    {
        isFast = newValue;
    }

    void static SetIsSingle(bool newValue)
    {
        isSingle = newValue;
    }

    static void DoRoutine(void);
};



//
// Функция, в потоке которой происходит выполнение Calculate
//
DWORD _stdcall StartCalculate(LPVOID lpThreadParameter)
{
    auto hWnd = *(HWND*)lpThreadParameter;
    while (true)
    {
        Calculate::DoRoutine();
        Redraw(hWnd);
    }
}


int _stdcall wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    LPCWSTR const className = L"Mondelbrot";

    Application::SetWndProc(WndProc);
    WNDCLASSEXW  wndClass{ sizeof(WNDCLASSEXW) };
    Application::FillStdWndClass(&wndClass, className);
    Application::RegisterWndClass(&wndClass);
    Videomemory::Initialize(XSIZE, YSIZE);

    auto hWnd = Application::MakeWindow(className,
                                        nCmdShow,
                                        L"Множество Мондельброта",
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

    auto calculateThread = CreateThread(
        nullptr,
        0,
        StartCalculate,
        &hWnd,
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

    Application::ProceedMessages();

    TerminateThread(calculateThread, 0);
    Videomemory::Delete();

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
                 yCenter =  0.0,
                 step    = (1.f / 800.f);

    static bool  isFast     = false;
    static bool  isSingle   = true;

    float const  
        shift       = 30.0,
        deltaScale  = 1.33f,
        xMin        = -1.42,
        xMax        = 0.33,
        yMin        = -1.43,
        yMax        = 1.43,
        stepMax     = 0.00691;

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
                                if (yCenter >= yMax)
                                    break;
                                yCenter += shift * step; 
                                Calculate::SetYCenter(yCenter);
                           })

            HANDLE_MESSAGE(VK_DOWN,
                           {
                                if (yCenter <= yMin)
                                    break;
                                yCenter -= shift * step;
                                Calculate::SetYCenter(yCenter);
                           })

            HANDLE_MESSAGE(VK_LEFT,
                           {
                                if (xCenter <= xMin)
                                    break;
                                xCenter -= shift * step;
                                Calculate::SetXCenter(xCenter);
                           })

            HANDLE_MESSAGE(VK_RIGHT,
                           {
                                if (xCenter >= xMax)
                                    break;
                                xCenter += shift * step;
                                Calculate::SetXCenter(xCenter);
                           })

            case VK_OEM_PLUS:
            HANDLE_MESSAGE(VK_ADD,
                           {
                                step /= deltaScale;
                                Calculate::SetStep(step);
                           })

            case VK_OEM_MINUS:
            HANDLE_MESSAGE(VK_SUBTRACT,
                           {
                                if (step >= stepMax)
                                    break;
                                step *= deltaScale; 
                                Calculate::SetStep(step);
                           })

            HANDLE_MESSAGE(VK_TAB,
                           {
                                isFast = !isFast;
                                Calculate::SetIsFast(isFast);
                           })

            HANDLE_MESSAGE(VK_SHIFT,
                           {
                                isSingle = !isSingle;
                                Calculate::SetIsSingle(isSingle);
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
                        MessageBox(hWnd, L"Fine release by JulesIMF!", L"Множество Мондельброта", MB_ICONINFORMATION);
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

    HDC hdc = Videomemory::Draw(hWnd); // вывод видеопамяти

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


void Calculate::DoRoutine(void)
{
    float     xCenter = Calculate::xCenter, yCenter  = Calculate::yCenter, step = Calculate::step;
    bool      isFast  = Calculate::isFast,  isSingle = Calculate::isSingle;

    typedef RGBQUAD(&scr_t)[scrY][scrX];    // Спасибо Деду за победу
    scr_t scr = (scr_t) * (Videomemory::image);

    for (int nCalcs = 0; nCalcs != (isSingle ? 1 : 10); nCalcs++)
    {
        // Обычное вычисление
        if (!isFast)
        {
            float xStart = xCenter - 400.f * step,
                  yStart = yCenter - 300.f * step;

            float xBegin = xStart, yBegin = yStart;

            for (int j = 0; j != scrY; j++, yBegin += step)
            {
                xBegin = xStart;
                for (int i = 0; i != scrX; i++, xBegin += step)
                {
                    float x = xBegin, y = yBegin;
                    int nTrans = 0;

                    for (; nTrans != maxTransforms; nTrans++)
                    {
                        auto xx = x * x,
                             xy = x * y,
                             yy = y * y;

                        if (xx + yy > maxSqRadius)
                            break;

                        x = xx - yy + xBegin;
                        y = xy + xy + yBegin;
                    }

                    auto myImage = Videomemory::image + 4 * (i + scrX * j);

                    float I = 1.f / (sqrtf((float)nTrans / (float)maxTransforms)) * 255.f;

                    BYTE c = (BYTE)I;
                    RGBQUAD color = (nTrans < maxTransforms) ? RGBQUAD{ (BYTE)(255 - c), (BYTE)((c % 2) * 64), c } : RGBQUAD{};
                    scr[j][i] = color;
                }
            }
        }

        // AVX
        else
        {
            float xStart = xCenter - 400.f * step,
                yStart = yCenter - 300.f * step;

            float xBegin = xStart, yBegin = yStart;

            __m256 radius_V = _mm256_set1_ps(maxSqRadius); // 8 раз maxSqRadius
            __m256 xStart_V = _mm256_add_ps(_mm256_set1_ps(xStart), _mm256_set_ps(step * 7.0,
                                                                                  step * 6.0,
                                                                                  step * 5.0,
                                                                                  step * 4.0,
                                                                                  step * 3.0,
                                                                                  step * 2.0,
                                                                                  step * 1.0,
                                                                                  0.0)); // при каждом новом y начинать надо с такого xBegin_V

            __m256 steps = _mm256_set1_ps(step * 8.0); // нужен для инкремента xBegin_V   
            const __m256  nmax = _mm256_set1_ps(maxTransforms);

            const __m256  _255 = _mm256_set1_ps(255.f);
            const __m256i _255i = _mm256_set1_epi32(255);
            const __m256i _1i = _mm256_set1_epi32(1);
            const __m256i _n1i = _mm256_set1_epi32(-1);
            const __m256i _n64i = _mm256_set1_epi32(255 - 64);

            for (int j = 0; j != scrY; j++, yBegin = yStart + j * step)
            {
                __m256 xBegin_V = xStart_V;
                __m256 yBegin_V = _mm256_set1_ps(yBegin);

                for (int i = 0; i != scrX; i += 8, xBegin_V = _mm256_add_ps(xBegin_V, steps))
                {
                    __m256 y_V = yBegin_V;
                    __m256 x_V = xBegin_V;

                    __m256i nTrans_V = _mm256_setzero_si256(); // здесь будет храниться вектор из количества преобразований, необходимых для вылета


                    //
                    // ЦЕЛЬ: Считает количество раз, необходимое для вылета
                    //
                    for (int nTrans = 0; nTrans != maxTransforms; nTrans++)
                    {
                        __m256 xx_V = _mm256_mul_ps(x_V, x_V); // вектор квадратов x
                        __m256 yy_V = _mm256_mul_ps(y_V, y_V); // вектор квадратов y

                        __m256i isLess_V = _mm256_castps_si256(_mm256_cmp_ps(_mm256_add_ps(xx_V, yy_V), radius_V, _CMP_LT_OQ)); // вектор 0 и -1 (-1 если точка не вылетела)
                        nTrans_V = _mm256_sub_epi32(nTrans_V, isLess_V);    // обновили nTrans_V

                        if (_mm256_testz_si256(isLess_V, _n1i))     // true, если ZF, то есть isLess_V содержит 0, т е все точки вылетели
                            break;

                        //
                        // Проверено - на черном не замедляет, а на цветном ускоряет в 2-3 раза
                        //

                        __m256 xy_V = _mm256_mul_ps(x_V, y_V);

                        x_V = _mm256_add_ps(_mm256_sub_ps(xx_V, yy_V), xBegin_V);
                        y_V = _mm256_add_ps(_mm256_add_ps(xy_V, xy_V), yBegin_V);
                    }

                    // стибрено у Деда, но один из корней заменен на rcp
                    __m256  I = _mm256_mul_ps(_mm256_rcp_ps(_mm256_sqrt_ps(_mm256_div_ps(_mm256_cvtepi32_ps(nTrans_V), nmax))), _255);

                    // RGBQUAD color = (pn[k] < maxTransforms) ? RGBQUAD{ (BYTE)(255 - c), (BYTE)(c % 2 * 64), c } : RGBQUAD{};

                    __m256i param = _mm256_and_si256(_mm256_cvtps_epi32(I), _255i);

                    __m256i color = _mm256_or_si256(_mm256_slli_epi32(param, 16),
                                                    _mm256_sub_epi32(_255i, param));

                    color = _mm256_or_si256(color,
                                            _mm256_slli_epi32(_mm256_and_si256(param, _1i), 16));

                    __m256i isIn = _mm256_xor_si256(_n1i,
                                                    _mm256_cmpeq_epi32(param, _255i));

                    color = _mm256_and_si256(isIn, color);

                    _mm256_store_si256((__m256i*)(&(scr[j][i])), color);

                    /*
                    *   Разница в цветах появляется из-за различия вычисления I -
                    *   в одном случае просто деление, в другом взятие обратного элемента.
                    *   Казалось бы, разница должна быть незначительной, но по факту...
                    */
                }
            }
        }
    }
}
