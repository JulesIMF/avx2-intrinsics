/*
Copyright (c) 2021  MIPT

Module Name:
    WinAPI_lib.h

Abstract:    
    
Author:
    JulesIMF

Last Edit:
    8.4.2021 21:31

Edit Notes:
    
*/

#ifndef JULESIMF_WINAPI_LIB
#define JULESIMF_WINAPI_LIB
#include <cassert>
#include "framework.h"

/// <summary>
/// VMResult - статус выполнения функции видеопамяти.
/// </summary>
enum VMresult {VMsucceed, VMfail};

/// <summary>
/// Эмуляция видеопамяти.
/// </summary>
class Videomemory
{
public:
    /// <summary>
    /// Указатель на буфер видеопамяти. Хранит RGBQUAD`ы
    /// </summary>
    inline static char* image = nullptr;

    /// <summary>
    /// xSize - размер окна по горизонтали,
    /// ySize - по вертикали
    /// </summary>
    inline static unsigned xSize, ySize;

    /// <summary>
    /// Устанавливает одну точку на экран
    /// </summary>
    /// <param name="x">Координата x</param>
    /// <param name="y">Координата y</param>
    /// <param name="pixel">Устанавливаемая точка</param>
    /// <returns>VMfail, если точка не влазит, VMsucceed иначе</returns>
    static VMresult SetPixel(unsigned x, unsigned y, RGBQUAD pixel)
    {
        if (x >= Videomemory::xSize || y >= Videomemory::ySize)
            return VMfail;

        *((RGBQUAD*)image + y * Videomemory::xSize + y) = pixel;
        return VMsucceed;
    }

    /// <summary>
    /// Создает буфер image
    /// </summary>
    /// <param name="newX">Размер экрана по горизонтали</param>
    /// <param name="newY">Размер экрана по вертикали</param>
    /// <returns>VMfail, если calloc вернул 0, VMsucceed иначе</returns>
    static VMresult Initialize(unsigned newX, unsigned newY)
    {
        image = (char*)calloc(newX * newY, sizeof(RGBQUAD));
        if (image == nullptr)
            return VMfail;

        Videomemory::xSize = newX, Videomemory::ySize = newY;

        return VMsucceed;
    }

    /// <summary>
    /// Удаляет буфер image.
    /// </summary>
    static void Delete(void)
    {
        free(image);
    }

    /// <summary>
    /// Перерисовывет буфер image в окно. Не заканчивает рисовать! Необходимо написать в конце EndPaint(hWnd, AMPERSAND ps);
    /// </summary>
    /// <param name="hWnd">Хендлер окна, в которое требуется перерисовать буфер</param>
    /// <returns>HDC устройства, в который рисовала эта функция.</returns>
    static HDC Draw(HWND hWnd)
    {
        PAINTSTRUCT ps = {};
        InvalidateRect(hWnd, NULL, true);

        HDC hdc = BeginPaint(hWnd, &ps);

        /*
        *   ПРОРИСОВКА БУФЕРА
        */
        BITMAPINFOHEADER bmih = {};
        bmih.biSize = sizeof(bmih);
        bmih.biWidth = xSize;
        bmih.biHeight = Videomemory::ySize;
        bmih.biPlanes = 1;
        bmih.biBitCount = 32;
        bmih.biCompression = BI_RGB;
        bmih.biSizeImage = 4 * Videomemory::xSize * Videomemory::ySize;

        HDC dc = GetDC(GetDesktopWindow());
        auto hBMP = CreateCompatibleBitmap(dc, Videomemory::xSize, Videomemory::ySize);
        SetDIBits(dc, hBMP, 0, (UINT)ySize, image, (BITMAPINFO*)&bmih, 0);
        ReleaseDC(GetDesktopWindow(), dc);

        RECT rc = {};
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));

        // получаем размеры картинки из битмэпа
        BITMAP bm = {};
        GetObject(hBMP, sizeof(bm), &bm);
        // нужен вспомогательный контекст
        HDC mdc = CreateCompatibleDC(hdc);

        // выбираем в вспомогательный контекст нужную картинку
        HBITMAP oldBitmap = (HBITMAP)SelectObject(mdc, hBMP);
        // копируем картинку на основной контекст
        int x = (rc.right - bm.bmWidth) / 2;
        int y = (rc.bottom - bm.bmHeight) / 2;

        // фень юань
        // диб секция



        BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, mdc, 0, 0, SRCCOPY);
        // возвращаем в вспомогательный контекст оригинальную 
        SelectObject(mdc, oldBitmap);
        // удаляем вспомогательный контекст
        DeleteDC(mdc);

        return hdc;
    }
};



/// <summary>
/// Класс, хранящий глобальные переменные, и предоставляющий стандартные функции для запуска Win32 приложения
/// </summary>
class Application
{
public:
    /// <summary>
    /// Регистрирует класс окна
    /// </summary>
    /// <param name="wcex">Указатель на регистрируемый класс</param>
    /// <returns></returns>
    static ATOM RegisterWndClass(WNDCLASSEXW* wcex)
    {
        return RegisterClassExW(wcex);
    }

    /// <summary>
    /// Заполняет класс окна стандартными значениями
    /// </summary>
    /// <param name="wcex">Указатель на структуру, которую надо заполнить</param>
    /// <param name="className">Имя класса окна</param>
    static void FillStdWndClass(WNDCLASSEXW* wcex, LPCWSTR className = nullptr)
    {
        wcex->cbSize        = sizeof(WNDCLASSEXW);
        wcex->style         = CS_HREDRAW | CS_VREDRAW;
        wcex->lpfnWndProc   = WndProc;
        wcex->cbClsExtra    = 0;
        wcex->cbWndExtra    = 0;
        wcex->hInstance     = hInst;
        wcex->hIcon         = LoadIcon(0, IDI_INFORMATION);
        wcex->hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wcex->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex->lpszMenuName  = 0;
        wcex->lpszClassName = className;
        wcex->hIconSm       = LoadIcon(0, IDI_INFORMATION);
    }

    /// <summary>
    /// Создает и показывает окно
    /// </summary>
    /// <param name="lpClassName">Имя класса окна</param>
    /// <param name="lpWindowName">Имя окна (title)</param>
    /// <param name="dwStyle">Стиль окна</param>
    /// <param name="nWidth">Ширина окна</param>
    /// <param name="nHeight">Высота окна</param>
    /// <param name="X">Начальное положение левой грани рамки</param>
    /// <param name="Y">Начальное положение верхней грани рамки</param>
    /// <param name="hWndParent">Хендлер родительского окна</param>
    /// <param name="hMenu">Хендлер меню</param>
    /// <param name="lpParam">Дополнительные параметры (см. MSDN)</param>
    /// <returns></returns>
    static HWND MakeWindow(_In_opt_ LPCWSTR lpClassName,
                           _In_     int     nCmdShow,
                           _In_opt_ LPCWSTR lpWindowName = L"Window",
                           _In_     DWORD   dwStyle      = (WS_OVERLAPPEDWINDOW),
                           _In_     int     nWidth       = 800,
                           _In_     int     nHeight      = 600,
                           _In_     int     X            = CW_USEDEFAULT,
                           _In_     int     Y            = 0,                    
                           _In_opt_ HWND    hWndParent   = nullptr,
                           _In_opt_ HMENU   hMenu        = nullptr,
                           _In_opt_ LPVOID lpParam       = nullptr)
    {
        auto hWnd = CreateWindowW(
            lpClassName,
            lpWindowName,
            dwStyle,
            X,
            Y,
            nWidth,
            nHeight,
            hWndParent,
            hMenu,
            hInst,
            lpParam
        );

        if (hWnd == nullptr)
            return hWnd;

        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);

        return hWnd;
    }

    /// <summary>
    /// Функция, реализующая цикл обработки сообщений
    /// </summary>
    /// <param name="hAccelTable">Хендлер таблицы акселераторов (горячих клавиш); nullptr по умолчанию</param>
    static void ProceedMessages(HACCEL hAccelTable = nullptr)
    {
        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    
    /// <summary>
    /// Устанавливает внутреннее поле WndProc. НЕ обновляет WndProc в окне
    /// </summary>
    /// <param name="newWndProc">Указатель на новую функцию обработки сообщений</param>
    static void SetWndProc(WNDPROC newWndProc)
    {
        WndProc = newWndProc;
    }


    /// <summary>
    /// Возвращает значение WndProc - указателя на функцию обработки сообщений
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    WNDPROC GetWndProc(void)
    {
        return WndProc;
    }

    static inline HINSTANCE hInst = nullptr;

protected:
    static inline decltype(WNDCLASSEXW::lpfnWndProc) WndProc = nullptr;
};

#ifndef HANDLE_MESSAGE
#define HANDLE_MESSAGE(message, body) case message: body; break;
#endif


#endif
