//// Intrinsic_1.cpp : Определяет точку входа для приложения.
////
//#define _CRT_SECURE_NO_WARNINGS
//#include "framework.h"
//#include "Intrinsic_2.h"
//#include "LoadBMP.h"
//#include <cstdio>
//#include <cmath>
//#include <cassert>
//#include <immintrin.h>
//
//#define MAX_LOADSTRING 100
//#define XSIZE          800                        // размер окна по горизонтали
//#define YSIZE          600                        // размер окна по вертикали
//
//
//static_assert(YSIZE % 8 == 0 && XSIZE % 8 == 0, "XSIZE and YSIZE must be multiples of 8!");
//
//
//// Глобальные переменные:
//HINSTANCE       hInst;                                // текущий экземпляр
//WCHAR           szTitle[MAX_LOADSTRING];              // Текст строки заголовка
//WCHAR           szWindowClass[MAX_LOADSTRING];        // имя класса главного окна
//namespace Video
//{
//    static char* image;
//}
//
//
//// Отправить объявления функций, включенных в этот модуль кода:
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//HWND                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
//
//struct RedrawParams
//{
//    HWND hWnd;
//};
//
//struct StartCalculateParams
//{
//    HWND  hWnd;
//    char* tableBuffer;
//    bool* needReload;
//};
//
//void         Redraw(RedrawParams, bool toUpdateFps = false);
//void         Calculate(bool switchAvx = false, 
//                              bool switchLoop = false, 
//                              bool switchBmp = false, 
//                              bool newXY = false, 
//                              int deltaX = 0, 
//                              int deltaY = 0, 
//                              BMP table = {}, 
//                              BMP cat = {});
//
//DWORD WINAPI StartCalculate(LPVOID lpThreadParameter)
//{
//    auto redrawParams = *(StartCalculateParams*)lpThreadParameter;
//    while (true)
//    {
//        Calculate();
//        Redraw({ redrawParams.hWnd });
//        if (*redrawParams.needReload)
//            memcpy(Video::image, redrawParams.tableBuffer, XSIZE * YSIZE * 4);
//    }
//}
//
//int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
//                      _In_opt_ HINSTANCE hPrevInstance,
//                      _In_ LPWSTR    lpCmdLine,
//                      _In_ int       nCmdShow)
//{
//    // Инициализация глобальных строк
//    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//    LoadStringW(hInstance, IDC_INTRINSIC2, szWindowClass, MAX_LOADSTRING);
//    MyRegisterClass(hInstance);
//
//    // Инициализация "видеопамяти"
//    Video::image = new char[4 * XSIZE * YSIZE];
//    
//    // Выполнить инициализацию приложения:
//    auto hWnd = InitInstance(hInstance, nCmdShow);
//    if (!hWnd)
//    {
//        return FALSE;
//    }
//
//    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INTRINSIC2));
//
//    MSG msg;
//
//    //
//    // Загрузим изображения
//    //
//    
//    FILE* table = fopen("Table.bmp", "rb");
//    FILE* cat   = fopen("DimaCat.bmp", "rb");
//    if (!table)
//    {
//        MessageBox(nullptr, L"Не удалось открыть Table.bmp", L"Кот", MB_ICONERROR);
//        return 1;
//    }
//
//    if (!cat)
//    {
//        MessageBox(nullptr, L"Не удалось открыть DimaCat.bmp", L"Кот", MB_ICONERROR);
//        return 2;
//    }
//
//    auto bmpTable = loadImage(table),
//         bmpCat   = loadImage(cat);
//
//    fclose(table);
//    fclose(cat);
//
//    if (!bmpTable.xSize)
//    {
//        MessageBox(nullptr, L"Изображение стола повреждено", L"Кот", MB_ICONERROR);
//        return 3;
//    }
//
//    if (bmpTable.xSize != XSIZE || bmpTable.ySize != YSIZE)
//    {
//        MessageBox(nullptr, L"Изображение стола имеет неправильные размеры (не 800х600)", L"Кот", MB_ICONERROR);
//        return 4;
//    }
//
//    if (!bmpCat.xSize)
//    {
//        MessageBox(nullptr, L"Изображение кота повреждено", L"Кот", MB_ICONERROR);
//        return 5;
//    }
//    
//    memcpy(Video::image, bmpTable.image, XSIZE * YSIZE * 4);        // загрузим стол в видеопамять
//    Calculate(false, false, true, false, 0, 0, bmpTable, bmpCat);   // передадим в Calculate изображения
//
//    bool needReload = false;    // здесь хранится флаг необходимости перезагрузки стола (вдруг кот переехал? мы же не хотим рисовать котом)
//    StartCalculateParams startParams = { hWnd, bmpTable.image, &needReload };
//
//    // Запустим поток, который будет рисовать кота
//    auto calculateThread = CreateThread(
//        nullptr,
//        0,
//        StartCalculate,
//        &startParams,
//        0,
//        0);
//
//    if (!calculateThread)
//    {
//        MessageBox(hWnd, L"Не удалось запустить поток рассчета изображения (calculateThread == nullptr)",
//                   L"Ошибка CreateThread",
//                   MB_ICONERROR);
//        return 5;
//    }
//
//    // Цикл основного сообщения:
//    while (GetMessage(&msg, nullptr, 0, 0))
//    {
//        if (msg.message == WM_KEYDOWN)
//            needReload = true;
//
//        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//    }
//
//    TerminateThread(calculateThread, 0);
//    delete[] Video::image;
//    free(bmpCat.image);
//    free(bmpTable.image);
//
//    return (int)msg.wParam;
//}
//
//
//
////
////  ФУНКЦИЯ: MyRegisterClass()
////
////  ЦЕЛЬ: Регистрирует класс окна.
////
//ATOM MyRegisterClass(HINSTANCE hInstance)
//{
//    WNDCLASSEXW wcex;
//
//    wcex.cbSize = sizeof(WNDCLASSEX);
//
//    wcex.style = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc = WndProc;
//    wcex.cbClsExtra = 0;
//    wcex.cbWndExtra = 0;
//    wcex.hInstance = hInstance;
//    wcex.hIcon = LoadIcon(0, IDI_INFORMATION);
//    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
//    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    wcex.lpszMenuName = /*MAKEINTRESOURCEW(IDC_INTRINSIC1);*/ 0;
//    wcex.lpszClassName = szWindowClass;
//    wcex.hIconSm = LoadIcon(0, IDI_INFORMATION);
//
//    return RegisterClassExW(&wcex);
//}
//
//
////
////   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
////
////   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
////
////   КОММЕНТАРИИ:
////
////        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
////        создается и выводится главное окно программы.
////
//HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
//{
//    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной
//
//    HWND hWnd = CreateWindowW(szWindowClass,
//                              L"Кот Димы Евтушенко",
//                              (WS_OVERLAPPEDWINDOW) ^ (WS_THICKFRAME | WS_MAXIMIZE | WS_MAXIMIZEBOX),
//                              CW_USEDEFAULT, 0,
//                              XSIZE, YSIZE,
//                              nullptr,
//                              nullptr,
//                              hInstance,
//                              nullptr);
//
//    if (!hWnd)
//    {
//        MessageBox(nullptr, L"Не удалось создать окно (hWnd == nullptr)", L"Ошибка CreateWindowW", MB_OK | MB_ICONERROR);
//        return hWnd;
//    }
//
//    ShowWindow(hWnd, nCmdShow);
//    UpdateWindow(hWnd);
//
//    return hWnd;
//}
//
////
////  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
////
////  WM_PAINT    - Отрисовка главного окна
////  WM_DESTROY  - отправить сообщение о выходе и вернуться
////
////
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    // 
//    // Эти переменные отвечают за расположение картинки
//    // Они хранятся здесь, т.к. в этой функции обработка клавиш
//    
//
//    switch (message)
//    {
//        /*
//        *   УПРАВЛЕНИЕ КЛАВИШАМИ
//        *
//        *       UP      - поднять вверх
//        *       DOWN    - опустить вниз
//        *       LEFT    - передвинуть налево
//        *       RIGHT   - передвинуть нараво
//        *       PLUS    - приблизиться
//        *       MINUS   - отдалиться
//        */
//    case WM_KEYDOWN:
//    {
//        switch (wParam)
//        {
//        case VK_UP:
//            Calculate(false, false, false, true, 0, 8);
//            break;
//
//        case VK_DOWN:
//            Calculate(false, false, false, true, 0, -8);
//            break;
//
//        case VK_LEFT:
//            Calculate(false, false, false, true, -8, 0);
//            break;
//
//        case VK_RIGHT:
//            Calculate(false, false, false, true, 8, 0);
//            break;
//
//        case VK_TAB:
//            Calculate(true);
//            break;
//
//        case VK_SHIFT:
//            //Calculate(false, true);
//            break;
//
//        case VK_SPACE:
//            Redraw({ hWnd }, true); // хотим переключить заморозку
//            break;
//
//        default:
//            break;
//        }
//
//        //Calculate(true, isFast, isSingle, xCenter, yCenter, step);
//    }
//    break;
//    case WM_PAINT:
//        //Redraw({ hWnd });
//        break;
//    case WM_DESTROY:
//        MessageBox(hWnd, L"Fine release by JulesIMF!", L"Кот Димы Евтушенко", MB_ICONINFORMATION);
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
//
//
////
////  ФУНКЦИЯ:    Redraw(HWND hWnd)
////
////  ЦЕЛЬ:       Перерисовывает буфер в окно и выводит FPS
////
//void Redraw(RedrawParams redrawParams, bool switchUpdateFps)
//{
//
//    static DWORD previousTime = 0;
//    static float fps = 0;
//    static bool  toUpdateFps = true;
//    PAINTSTRUCT ps;
//    auto hWnd = redrawParams.hWnd;
//    InvalidateRect(hWnd, NULL, true);
//
//    if (switchUpdateFps)
//        toUpdateFps = !toUpdateFps;
//
//    HDC hdc = BeginPaint(hWnd, &ps);
//
//    /*
//    *   ПРОРИСОВКА БУФЕРА
//    */
//    BITMAPINFOHEADER bmih = {};
//    bmih.biSize = sizeof(bmih);
//    bmih.biWidth = XSIZE;
//    bmih.biHeight = YSIZE;
//    bmih.biPlanes = 1;
//    bmih.biBitCount = 32;
//    bmih.biCompression = BI_RGB;
//    bmih.biSizeImage = 4 * XSIZE * YSIZE;
//
//    HDC dc = GetDC(GetDesktopWindow());
//    auto hBMP = CreateCompatibleBitmap(dc, XSIZE, YSIZE);
//    SetDIBits(dc, hBMP, 0, (UINT)YSIZE, Video::image, (BITMAPINFO*)&bmih, 0);
//    ReleaseDC(GetDesktopWindow(), dc);
//
//    RECT rc;
//    GetClientRect(hWnd, &rc);
//    FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
//
//    // получаем размеры картинки из битмэпа
//    BITMAP bm = {};
//    GetObject(hBMP, sizeof(bm), &bm);
//    // нужен вспомогательный контекст
//    HDC mdc = CreateCompatibleDC(hdc);
//
//    // выбираем в вспомогательный контекст нужную картинку
//    HBITMAP oldBitmap = (HBITMAP)SelectObject(mdc, hBMP);
//    // копируем картинку на основной контекст
//    int x = (rc.right - bm.bmWidth) / 2;
//    int y = (rc.bottom - bm.bmHeight) / 2;
//
//
//
//    BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, mdc, 0, 0, SRCCOPY);
//    // возвращаем в вспомогательный контекст оригинальную 
//    SelectObject(mdc, oldBitmap);
//    // удаляем вспомогательный контекст
//    DeleteDC(mdc);
//
//    /*
//    *   ВЫВОД FPS
//    */
//    DWORD currentTime = GetTickCount();
//
//    int nOut = 0;
//    wchar_t bufferForFps[32];
//    char    bufferForPrintf[32];
//    if (toUpdateFps)
//        fps = 1000.f / (currentTime - previousTime);
//
//    snprintf(bufferForPrintf, 31, "%.2f", fps);
//    nOut = wsprintf(bufferForFps, L"%S", bufferForPrintf);
//    assert(nOut);
//    previousTime = currentTime;
//    TextOut(hdc, 0, 0, bufferForFps, nOut);
//
//    // UpdateWindow(hWnd);
//    EndPaint(hWnd, &ps);
//}
//
//
////
//// Раз в Интеле не сделали такого интринсика, 
//// то придется брать ситуацию в свои руки
////
//inline __m256i _mm256_mulhi_epu8(__m256i a, __m256i b)
//{
//    __m256i aCopy = a, bCopy = b;
//
//    __m256i maskVast        = _mm256_set_epi8(-1, 15, -1, 14, -1, 13, -1, 12,
//                                              -1, 11, -1, 10, -1, 9,  -1, 8,
//                                              -1, 7,  -1, 6,  -1, 5,  -1, 4,
//                                              -1, 3,  -1, 2,  -1, 1,  -1, 0);
//
//    __m256i maskConsolidate = _mm256_set_epi8(15, 13, 11,  9,  7,  5,  3,  1,
//                                              -1, -1, -1, -1, -1, -1, -1, -1,
//                                              -1, -1, -1, -1, -1, -1, -1, -1,
//                                              15, 13, 11,  9,  7,  5,  3,  1);
//
//
//    __m256i aLowDup  = _mm256_broadcastsi128_si256(_mm256_castsi256_si128(a));
//    __m256i bLowDup  = _mm256_broadcastsi128_si256(_mm256_castsi256_si128(b));
//
//    __m256i aHighDup = a; *((__m128i*)(&aHighDup)) = *((__m128i*)(&aHighDup) + 1); // почему нельзя красиво запихать верхнюю часть ymm в xmm?((
//    __m256i bHighDup = b; *((__m128i*)(&bHighDup)) = *((__m128i*)(&bHighDup) + 1); //
//
//
//
//    __m256i aLowVast  = _mm256_shuffle_epi8(aLowDup,  maskVast); // 
//    __m256i bLowVast  = _mm256_shuffle_epi8(bLowDup,  maskVast); // расставляем широко числа
//    __m256i aHighVast = _mm256_shuffle_epi8(aHighDup, maskVast); // 
//    __m256i bHighVast = _mm256_shuffle_epi8(bHighDup, maskVast); //
//
//    aLowVast  = _mm256_mullo_epi16(aLowVast,  bLowVast);  // перемножаем
//    aHighVast = _mm256_mullo_epi16(aHighVast, bHighVast); //
//
//    aLowVast  = _mm256_shuffle_epi8(aLowVast,  maskConsolidate); // консолидируем, то есть собираем вместе старшие разряды произведений
//    aHighVast = _mm256_shuffle_epi8(aHighVast, maskConsolidate); //
//    
//    *(__m128i*)(&aLowVast)        = _mm_or_si128(_mm256_castsi256_si128(aLowVast),  *((__m128i*)(&aLowVast)  + 1)); // объединяем старший ymm и xmm
//    *((__m128i*)(&aHighVast) + 1) = _mm_or_si128(_mm256_castsi256_si128(aHighVast), *((__m128i*)(&aHighVast) + 1)); //
//    *(__m128i*)(&aHighVast)       = _mm_setzero_si128(); // зануляем то что уже не надо
//    *((__m128i*)(&aLowVast) + 1)  = _mm_setzero_si128(); //
//    
//
//    auto ans = _mm256_or_si256(aHighVast, aLowVast); // объединяем то что получилось
//
//    return ans;
//}
//
//
//void Calculate(bool switchAvx, bool switchLoop, bool switchBmp, bool newXY, int deltaX, int deltaY, BMP newTable, BMP newCat)
//{
//    using Video::image;
//
//    static bool isAvx = true;
//    static int  loops = 50;
//    static BMP  table = {},
//                cat   = {};
//
//    static int  xCatStatic = 128,
//                yCatStatic = 144;
//
//    // Обработаем команды переключения
//    if (switchAvx)
//        isAvx = !isAvx;
//
//    if (switchLoop)
//        loops = 750 - loops;
//
//    if (switchBmp)
//        table = newTable,
//        cat   = newCat;
//
//    if (newXY)
//    {
//        int newX = xCatStatic + deltaX,
//            newY = yCatStatic + deltaY;
//
//        if (newX % 8 != 0)
//        {
//            MessageBox(nullptr, L"Неверный xCat (не кратен 8)!", L"Кот Димы Евтушенко", MB_ICONERROR);
//            exit(10);
//        }
//
//        if (newX >= 0 && newX + cat.xSize <= XSIZE)
//            xCatStatic = newX;
//
//        if (newY >= 0 && newY + cat.ySize <= YSIZE)
//            yCatStatic = newY;
//    }
//
//    if (switchAvx || switchLoop || switchBmp || newXY)
//        return;
//    
//
//    int xCat = xCatStatic, yCat = yCatStatic;
//    if (isAvx)
//    {
//        for (int nCalc = 0; nCalc != loops; nCalc++)
//        {
//            __m256i _m1      = _mm256_set1_epi32(-1);
//            __m256i alMask   = _mm256_set_epi8(15, 15, 15, 15,   // маска для размножения альфа-канала
//                                               11, 11, 11, 11,
//                                               7,  7,  7,  7,
//                                               3,  3,  3,  3,
//                                               15, 15, 15, 15,
//                                               11, 11, 11, 11,
//                                               7,  7,  7,  7,
//                                               3,  3,  3,  3);
//            int catIndex = 0;
//
//            for (int j = 0; j != cat.ySize; j++) // уважаем кэш
//            {
//                int index = (XSIZE * (j + yCat) + xCat);
//
//                int i = 0;                                                      // ширина кота может быть не кратна 8, 
//                for (; i + 7 <= cat.xSize; i += 8, index += 8, catIndex += 8)   // поэтому если что, надо будет дообработать снизу
//                {
//                    __m256i rgbTable  = *(__m256i*)(table.image + index    * sizeof(RGBQUAD)),
//                            rgbCat    = *(__m256i*)(cat.image   + catIndex * sizeof(RGBQUAD));
//
//
//                    __m256i alphaCat   = _mm256_shuffle_epi8(rgbCat, alMask);       // получаем пачки по 4 альфа-канала для кота
//                    __m256i alphaTable = _mm256_sub_epi8(_m1, alphaCat);            // для стола
//
//                    __m256i blendCat   = _mm256_mulhi_epu8(alphaCat,   rgbCat);     // это замешанный кот
//                    __m256i blendTable = _mm256_mulhi_epu8(alphaTable, rgbTable);   // а это замешанный стол
//
//                    auto res = _mm256_add_epi8(blendCat, blendTable);               // в результате просто надо сложить
//                    *(__m256i*)((RGBQUAD*)image + index) = res;
//                }
//                                                                                    // а это обещанная дообработка того, 
//                for (; i != cat.xSize; i++, index++)                                // не поместилось в кратные 8 куски
//                {
//                    RGBQUAD rgbTable = *((RGBQUAD*)table.image + index),
//                            rgbCat   = *((RGBQUAD*)cat.image   + catIndex++);
//
//                    RGBQUAD result =
//                    {
//                        (rgbTable.rgbBlue  * (255 - rgbCat.rgbReserved) + rgbCat.rgbBlue  * rgbCat.rgbReserved) >> 8,
//                        (rgbTable.rgbGreen * (255 - rgbCat.rgbReserved) + rgbCat.rgbGreen * rgbCat.rgbReserved) >> 8,
//                        (rgbTable.rgbRed   * (255 - rgbCat.rgbReserved) + rgbCat.rgbRed   * rgbCat.rgbReserved) >> 8,
//                        255
//                    };
//
//                    *((RGBQUAD*)image + index) = result;
//                }
//
//            }
//        }
//    }
//
//    else
//    {
//    notAvx:
//        for (int nCalc = 0; nCalc != loops; nCalc++)
//        {
//            int catIndex = 0;
//            for (int j = 0; j != cat.ySize; j++)
//            {
//                int index = (XSIZE * (j + yCat) + xCat);
//                for (int i = 0; i != cat.xSize; i++, index++)
//                {
//                    RGBQUAD rgbTable = *((RGBQUAD*)table.image + index),
//                            rgbCat   = *((RGBQUAD*)cat.image   + catIndex++);
//
//                    RGBQUAD result   = 
//                    {
//                        ( rgbTable.rgbBlue  * (255 - rgbCat.rgbReserved) + rgbCat.rgbBlue  * rgbCat.rgbReserved ) >> 8,
//                        ( rgbTable.rgbGreen * (255 - rgbCat.rgbReserved) + rgbCat.rgbGreen * rgbCat.rgbReserved ) >> 8,
//                        ( rgbTable.rgbRed   * (255 - rgbCat.rgbReserved) + rgbCat.rgbRed   * rgbCat.rgbReserved ) >> 8,
//                        255
//                    };
//
//                    *((RGBQUAD*)image + index) = result;
//                }
//            }
//        }
//    }
//}