/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMSTAT.C                                   **
**                                                                            **
**  This module contains the procedures to handle the status display          **
**  window.                                                                   **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"

/* LOCAL STORAGE -------------------------------------------------------------*/

static DWORD dwStartTick = 0;
static BOOL  bRunTimer = FALSE;

/*----------------------------------------------------------------------------*/
VOID PRVFUNC DrawStTime(void)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    char         szBuf[32];
    DWORD    dwHours, dwMinutes, dwSeconds;

    if (!bRunTimer)
        return;

    dwSeconds = (GetTickCount() - dwStartTick) / 1000;

    dwMinutes = dwSeconds / 60;
    dwSeconds = dwSeconds % 60;

    dwHours = dwMinutes / 60;
    dwMinutes = dwMinutes % 60;

    wsprintf(szBuf, "%0.2ld:%0.2ld:%0.2ld", dwHours, dwMinutes, dwSeconds);

    SetStatus(WID_STTIME, szBuf);
}


/*----------------------------------------------------------------------------*/
VOID PRVFUNC Status_OnCommand
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
BOOL PRVFUNC Status_OnCreate
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit windows creation stuff.                      */
/*                                        */
(HWND           hWnd,         /* Window that received the command.    */
 LPCREATESTRUCT) /* lpCreateStruct */ /* Create structure.            */
/*----------------------------------------------------------------------------*/

{
    int idControl;

    for (idControl = WID_STCOMM; idControl <= WID_STTIME; idControl++)
        CrtSubWnd(szClsKermText, NULL, hWnd, idControl, hAppInst);

    DrawStTime();

    return(TRUE);
}


/*----------------------------------------------------------------------------*/
void PRVFUNC Status_OnSize
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit window size change.                          */
/*                                        */
(HWND      hWnd,     /* Window that received the command.              */
 UINT      X(state), /* Window state.                          */
 int       cx,       /* New window width.                      */
 int       cy)       /* New window height.                     */
/*----------------------------------------------------------------------------*/

{
    int  xStWnd, nItems;
    HDC  hDC;
    HDWP hdwp;

    hDC = GetDC(hWnd);

    xStWnd = GetTextWidth(hDC, "99:99:99") + (xSysChar * 2) + 4;

    ReleaseDC(hWnd, hDC);

    hdwp = BeginDeferWindowPos(5);

    hdwp = DefMovSubWnd(hdwp, hWnd, WID_STTIME,
            cx - xStWnd + 2, 2, xStWnd - 4, cy - 4);
    cx -= xStWnd;

    nItems = min(4, max(1, cx / (xSysChar * 20)));
    xStWnd = cx / nItems;

    if (nItems >= 1) {
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STCOMM,
                            cx - xStWnd + 2, 2, xStWnd - 4, cy - 4);
        cx -= xStWnd;
    }
    else
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STCOMM, 0, 0, 0, 0);

    if (nItems >= 2) {
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STPROT,
                            cx - xStWnd + 2, 2, xStWnd - 4, cy - 4);
        cx -= xStWnd;
    }
    else
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STPROT, 0, 0, 0, 0);

    if (nItems >= 3) {
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STEMUL,
                            cx - xStWnd + 2, 2, xStWnd - 4, cy - 4);
        cx -= xStWnd;
    }
    else
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STEMUL, 0, 0, 0, 0);

    if (nItems >= 4) {
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STDEV,
                            0 + 2, 2, cx - 4, cy - 4);
    }
    else
        hdwp = DefMovSubWnd(hdwp, hWnd, WID_STDEV, 0, 0, 0, 0);

    EndDeferWindowPos(hdwp);
}


/*----------------------------------------------------------------------------*/
void PRVFUNC Status_OnPaint
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit window size change.                          */
/*                                        */
(HWND   hWnd)   /* Window that received the command.                  */
/*----------------------------------------------------------------------------*/

{
    PAINTSTRUCT ps;
    HDC hDC;
    RECT rect;

    GetClientRect(hWnd, &rect);
    hDC = BeginPaint(hWnd, &ps);

//  DrawEdge(hDC, &rect, EDGE_ETCHED, BF_TOP | BF_BOTTOM);

    EndPaint(hWnd, &ps);
}


/*---------------------------------------------------------------------------*/
LONG CALLBACK __export StatusWndProc
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

        HANDLE_MSG(hWnd, WM_COMMAND, Status_OnCommand);

        HANDLE_MSG(hWnd, WM_CREATE, Status_OnCreate);

        HANDLE_MSG(hWnd, WM_SIZE, Status_OnSize);

        HANDLE_MSG(hWnd, WM_PAINT, Status_OnPaint);

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return(0L);
}


/*----------------------------------------------------------------------------*/
BOOL PUBFUNC StatusInit(HINSTANCE hInstance)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    WNDCLASS WndClass;

    memset(&WndClass, 0, sizeof(WndClass));

    WndClass.lpszClassName    = szClsKermStatus;
    WndClass.lpfnWndProc          = StatusWndProc;
    WndClass.style        = NULL;
    WndClass.hInstance        = hInstance;
//    WndClass.hbrBackground        = NULL;
    WndClass.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.hCursor              = LoadCursor(NULL, IDC_ARROW);

    return(RegisterClass(&WndClass) != NULL);
}


/*----------------------------------------------------------------------------*/
VOID PUBFUNC SetStatus(int nItem, LPSTR lpszItem)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    HWND hWnd;

    hWnd = hAppWnd;
    hWnd = hWnd ? GetDlgItem(hWnd, WID_STATUS) : NULL;
    hWnd = hWnd ? GetDlgItem(hWnd, nItem) : NULL;

    if (hWnd)
    SetText(hWnd, NULL, lpszItem);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC SetTimer(int nStatus)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    switch (nStatus) {
        case ST_RESET:
            dwStartTick = 0;
            SetStatus(WID_STTIME, "00:00:00");
            break;

        case ST_START:
            dwStartTick = GetTickCount();
            bRunTimer = TRUE;
            break;

        case ST_STOP:
            dwStartTick = 0;
            bRunTimer = FALSE;
            break;
    }
}

/*----------------------------------------------------------------------------*/
void PUBFUNC StatusUpdate
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle timer interupts for status window.                        */
/*                                        */
(void)              /* No parms.                    */
/*----------------------------------------------------------------------------*/

{
    DrawStTime();
}