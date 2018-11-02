/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**               KERMTEXT.C                   **
**                                                                            **
**  This module contains the procedures to handle the private text window     **
**  class.                                    **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"

/*----------------------------------------------------------------------------*/
VOID PRVFUNC Text_OnCommand
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle menu commands.                              */
/*                                        */
(HWND   hWnd,     /* Window that received the command.                */
 int    Id,   /* Command (wParam of WM_COMMAND message).              */
 HWND   hWndCtl,   /* Parameter (lParam of WM_COMMAND message).           */
 UINT   wNotify)   /* Parameter (lParam of WM_COMMAND message).           */
/*----------------------------------------------------------------------------*/

{
    FORWARD_WM_COMMAND(hWnd, Id, hWndCtl, wNotify, DefWindowProc);
}


/*----------------------------------------------------------------------------*/
BOOL PRVFUNC Text_OnCreate
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit windows creation stuff.                      */
/*                                        */
(HWND       hWnd,       /* Window that received the command.          */
 LPCREATESTRUCT lpCreateStruct) /* Create structure.                  */
/*----------------------------------------------------------------------------*/

{
    FORWARD_WM_CREATE(hWnd, lpCreateStruct, DefWindowProc);

    return(TRUE);
}


/*----------------------------------------------------------------------------*/
void PRVFUNC Text_OnSize
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit window size change.                          */
/*                                        */
(HWND   hWnd,   /* Window that received the command.                  */
 UINT   state,  /* Window state.                          */
 int    cx, /* New window width.                          */
 int    cy) /* New window height.                         */
/*----------------------------------------------------------------------------*/

{
    FORWARD_WM_SIZE(hWnd, state, cx, cy, DefWindowProc);
}


/*----------------------------------------------------------------------------*/
void PRVFUNC Text_OnPaint
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit window size change.                          */
/*                                        */
(HWND   hWnd)   /* Window that received the command.                  */
/*----------------------------------------------------------------------------*/

{
    PAINTSTRUCT ps;
    HDC hDC;
    HPEN hPen;
    char sWndText[80];
    int i;
    RECT rect;

    i = GetWindowText(hWnd, sWndText, 80);
    GetClientRect(hWnd, &rect);
    hDC = BeginPaint(hWnd, &ps);

    SaveDC(hDC);
    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));

#if 1
    SelectObject(hDC, hPen);
    MoveToEx(hDC, (int)rect.left, (int)rect.top, NULL);
    LineTo(hDC, (int)rect.right, (int)rect.top);
    MoveToEx(hDC, (int)rect.left, (int)rect.top, NULL);
    LineTo(hDC, (int)rect.left, (int)rect.bottom);

    /*  Add shadow ??? */

    SelectObject(hDC, GetStockObject(WHITE_PEN));
    MoveToEx(hDC, (int)rect.left + 1, (int)rect.bottom - 1, NULL);
    LineTo(hDC, (int)rect.right, (int)rect.bottom - 1);
    MoveToEx(hDC, (int)rect.right - 1, (int)rect.top + yBord, NULL);
    LineTo(hDC, (int)rect.right - 1, (int)rect.bottom);
#endif

#if 0
    SelectObject(hDC, hPen);
    MoveToEx(hDC, rect.left, rect.top, NULL);
    LineTo(hDC, rect.right - 2, rect.top);
    LineTo(hDC, rect.right - 2, rect.bottom - 2);
    LineTo(hDC, rect.left, rect.bottom - 2);
//  LineTo(hDC, rect.left, rect.top);

    SelectObject(hDC, GetStockObject(WHITE_PEN));
    MoveToEx(hDC, rect.left + 1, rect.bottom - 3, NULL);
//  LineTo(hDC, rect.left + 1, rect.top + 1);
    MoveToEx(hDC, rect.left + 1, rect.top + 1, NULL);
    LineTo(hDC, rect.right - 3, rect.top + 1);
    MoveToEx(hDC, rect.left, rect.bottom - 1, NULL);
    LineTo(hDC, rect.right - 1, rect.bottom - 1);
    LineTo(hDC, rect.right - 1, rect.top);
#endif

//  DrawEdge(hDC, &rect, BDR_SUNKENOUTER, BF_RECT);



    /* */

    SetText(hWnd, hDC, sWndText);

    RestoreDC(hDC, -1);
    DeleteObject(hPen);

    EndPaint(hWnd, &ps);
}


/*---------------------------------------------------------------------------*/
LONG CALLBACK __export TextWndProc
/*---------------------------------------------------------------------------*/
/*                                       */
/* Window procedure for the base application window.                 */
/*                                       */
(HWND     hWnd,     /* Handle to window receiving message            */
 UINT     message,  /* Message number                        */
 WPARAM   wParam,   /* First Message Parameter (UINT)                */
 LPARAM   lParam)   /* Second Message Parameter (LONG)               */
/*---------------------------------------------------------------------------*/

{
    switch (message) {

    HANDLE_MSG(hWnd, WM_COMMAND, Text_OnCommand);

    HANDLE_MSG(hWnd, WM_CREATE, Text_OnCreate);

    HANDLE_MSG(hWnd, WM_SIZE, Text_OnSize);

    HANDLE_MSG(hWnd, WM_PAINT, Text_OnPaint);

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return(0L);
}


/*----------------------------------------------------------------------------*/
BOOL PUBFUNC TextInit(HINSTANCE hInstance)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    WNDCLASS WndClass;

    memset(&WndClass, 0, sizeof(WndClass));

    WndClass.lpszClassName    = szClsKermText;
    WndClass.lpfnWndProc      = TextWndProc;
    WndClass.style        = CS_HREDRAW;
    WndClass.hInstance        = hInstance;
    WndClass.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.hCursor              = LoadCursor(NULL, IDC_ARROW);

    return(RegisterClass(&WndClass) != NULL);
}


/*----------------------------------------------------------------------------*/
VOID PUBFUNC SetText(HWND hWnd, HDC hWndDC, LPSTR lpszText)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    RECT       rect;
    HDC        hDC;
    LOGFONT    lf;
    HFONT      hFont;
    TEXTMETRIC tm;
    int        xTextChar, yTextChar;

    SetWindowText(hWnd, lpszText);

    GetClientRect(hWnd, &rect);

    if (hWndDC == NULL)
        hDC = GetDC(hWnd);
    else
        hDC = hWndDC;

    SaveDC(hDC);
    SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
    SetTextColor(hDC, GetSysColor(COLOR_BTNTEXT));

    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = ySysChar;
    lstrcpy((PSTR)lf.lfFaceName, "Helv");

    hFont = CreateFontIndirect(&lf);
    SelectObject(hDC, hFont);

    GetTextMetrics(hDC, &tm);

#ifdef _WIN32
    xTextChar = (int)tm.tmAveCharWidth;
    yTextChar = (int)tm.tmHeight;
#else
    xTextChar = tm.tmAveCharWidth;
    yTextChar = tm.tmHeight;
#endif

    InflateRect(&rect, -xTextChar, -2);

//    rect.left += xTextChar;
//    rect.right -= xTextChar;

#ifdef _WIN32
    ExtTextOut(hDC, (int)rect.left, (yMenu - yTextChar + 1) / 2,
               ETO_CLIPPED | ETO_OPAQUE, &rect,
               lpszText, lstrlen(lpszText), NULL);
#else
    ExtTextOut(hDC, rect.left, (yMenu - yTextChar + 1) / 2,
               ETO_CLIPPED | ETO_OPAQUE, &rect,
               lpszText, lstrlen(lpszText), NULL);
#endif

    RestoreDC(hDC, -1);

    DeleteObject(hFont);

    if (hWndDC == NULL)
        ReleaseDC(hWnd, hDC);
}


/*----------------------------------------------------------------------------*/
int PUBFUNC GetTextWidth(HDC hDC, LPSTR lpszText)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    LOGFONT lf;
    HFONT   hFont, hPrevFont;
    SIZE    sz;

    if (hDC == NULL)
    return(0);

    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = ySysChar;
    lstrcpy((PSTR)lf.lfFaceName, "Helv");

    hFont = CreateFontIndirect(&lf);
    hPrevFont = (HFONT)SelectObject(hDC, hFont);

    GetTextExtentPoint(hDC, lpszText, lstrlen(lpszText), &sz);

    SelectObject(hDC, hPrevFont);
    DeleteObject(hFont);

    return((int)sz.cx);
}
