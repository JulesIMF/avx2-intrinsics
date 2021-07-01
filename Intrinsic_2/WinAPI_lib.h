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
/// VMResult - ������ ���������� ������� �����������.
/// </summary>
enum VMresult {VMsucceed, VMfail};

/// <summary>
/// �������� �����������.
/// </summary>
class Videomemory
{
public:
    /// <summary>
    /// ��������� �� ����� �����������. ������ RGBQUAD`�
    /// </summary>
    inline static char* image = nullptr;

    /// <summary>
    /// xSize - ������ ���� �� �����������,
    /// ySize - �� ���������
    /// </summary>
    inline static unsigned xSize, ySize;

    /// <summary>
    /// ������������� ���� ����� �� �����
    /// </summary>
    /// <param name="x">���������� x</param>
    /// <param name="y">���������� y</param>
    /// <param name="pixel">��������������� �����</param>
    /// <returns>VMfail, ���� ����� �� ������, VMsucceed �����</returns>
    static VMresult SetPixel(unsigned x, unsigned y, RGBQUAD pixel)
    {
        if (x >= Videomemory::xSize || y >= Videomemory::ySize)
            return VMfail;

        *((RGBQUAD*)image + y * Videomemory::xSize + y) = pixel;
        return VMsucceed;
    }

    /// <summary>
    /// ������� ����� image
    /// </summary>
    /// <param name="newX">������ ������ �� �����������</param>
    /// <param name="newY">������ ������ �� ���������</param>
    /// <returns>VMfail, ���� calloc ������ 0, VMsucceed �����</returns>
    static VMresult Initialize(unsigned newX, unsigned newY)
    {
        image = (char*)calloc(newX * newY, sizeof(RGBQUAD));
        if (image == nullptr)
            return VMfail;

        Videomemory::xSize = newX, Videomemory::ySize = newY;

        return VMsucceed;
    }

    /// <summary>
    /// ������� ����� image.
    /// </summary>
    static void Delete(void)
    {
        free(image);
    }

    /// <summary>
    /// ������������� ����� image � ����. �� ����������� ��������! ���������� �������� � ����� EndPaint(hWnd, AMPERSAND ps);
    /// </summary>
    /// <param name="hWnd">������� ����, � ������� ��������� ������������ �����</param>
    /// <returns>HDC ����������, � ������� �������� ��� �������.</returns>
    static HDC Draw(HWND hWnd)
    {
        PAINTSTRUCT ps = {};
        InvalidateRect(hWnd, NULL, true);

        HDC hdc = BeginPaint(hWnd, &ps);

        /*
        *   ���������� ������
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

        // �������� ������� �������� �� �������
        BITMAP bm = {};
        GetObject(hBMP, sizeof(bm), &bm);
        // ����� ��������������� ��������
        HDC mdc = CreateCompatibleDC(hdc);

        // �������� � ��������������� �������� ������ ��������
        HBITMAP oldBitmap = (HBITMAP)SelectObject(mdc, hBMP);
        // �������� �������� �� �������� ��������
        int x = (rc.right - bm.bmWidth) / 2;
        int y = (rc.bottom - bm.bmHeight) / 2;

        // ���� ����
        // ��� ������



        BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, mdc, 0, 0, SRCCOPY);
        // ���������� � ��������������� �������� ������������ 
        SelectObject(mdc, oldBitmap);
        // ������� ��������������� ��������
        DeleteDC(mdc);

        return hdc;
    }
};



/// <summary>
/// �����, �������� ���������� ����������, � ��������������� ����������� ������� ��� ������� Win32 ����������
/// </summary>
class Application
{
public:
    /// <summary>
    /// ������������ ����� ����
    /// </summary>
    /// <param name="wcex">��������� �� �������������� �����</param>
    /// <returns></returns>
    static ATOM RegisterWndClass(WNDCLASSEXW* wcex)
    {
        return RegisterClassExW(wcex);
    }

    /// <summary>
    /// ��������� ����� ���� ������������ ����������
    /// </summary>
    /// <param name="wcex">��������� �� ���������, ������� ���� ���������</param>
    /// <param name="className">��� ������ ����</param>
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
    /// ������� � ���������� ����
    /// </summary>
    /// <param name="lpClassName">��� ������ ����</param>
    /// <param name="lpWindowName">��� ���� (title)</param>
    /// <param name="dwStyle">����� ����</param>
    /// <param name="nWidth">������ ����</param>
    /// <param name="nHeight">������ ����</param>
    /// <param name="X">��������� ��������� ����� ����� �����</param>
    /// <param name="Y">��������� ��������� ������� ����� �����</param>
    /// <param name="hWndParent">������� ������������� ����</param>
    /// <param name="hMenu">������� ����</param>
    /// <param name="lpParam">�������������� ��������� (��. MSDN)</param>
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
    /// �������, ����������� ���� ��������� ���������
    /// </summary>
    /// <param name="hAccelTable">������� ������� ������������� (������� ������); nullptr �� ���������</param>
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
    /// ������������� ���������� ���� WndProc. �� ��������� WndProc � ����
    /// </summary>
    /// <param name="newWndProc">��������� �� ����� ������� ��������� ���������</param>
    static void SetWndProc(WNDPROC newWndProc)
    {
        WndProc = newWndProc;
    }


    /// <summary>
    /// ���������� �������� WndProc - ��������� �� ������� ��������� ���������
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
