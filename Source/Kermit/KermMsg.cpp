/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMMSG.C                                    **
**                                                                            **
**                      Version 0.50 - Developmental                          **
**                     Requires Microsoft Windows 2.X                         **
**                         Author: Wayne Warthen                              **
**                                                                            **
**  This module contains the functions and window procedures to handle the    **
**  message window at the bottom of the screen.                               **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermprot.h"

#include <dos.h>

/* LOCAL DATA ----------------------------------------------------------------*/

static char szStatusMsg [5] [80];

static char * MonthNames [] = {"Jan", "Feb", "Mar", "Apr",
                               "May", "Jun", "Jul", "Aug",
                               "Sep", "Oct", "Nov", "Dec"};


/*----------------------------------------------------------------------------*/
VOID PRVFUNC FormatTimeStr
/*                                        */
/* Update the current time display.                       */
/*                                        */
/*----------------------------------------------------------------------------*/
(LPSTR  lpszTimeStr)      /* Buffer to put time string in                     */
/*----------------------------------------------------------------------------*/

{
#ifdef _WIN32
    SYSTEMTIME lpst;
#else
    struct dosdate_t dd;
    struct dostime_t dt;
#endif

#ifdef _WIN32
    GetLocalTime(&lpst);

    wsprintf(lpszTimeStr, "%s %i - %i:%0.2i:%0.2i",
         (LPSTR)MonthNames[lpst.wMonth - 1], lpst.wDay,
             lpst.wHour, lpst.wMinute, lpst.wSecond);
#else
    _dos_getdate(&dd);
    _dos_gettime(&dt);

    wsprintf(lpszTimeStr, "%s %i - %i:%0.2i:%0.2i",
             (LPSTR)MonthNames[dd.month - 1], dd.day,
             dt.hour, dt.minute, dt.second);
#endif
}

/*----------------------------------------------------------------------------*/
void PRVFUNC UpdateWindowPos
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit window size change.                          */
/*                                        */
(HWND      hWnd,     /* Window that received the command.             */
 int       cxw,      /* New window width.                     */
 int       cyw)      /* New window height.                    */
/*----------------------------------------------------------------------------*/

{
    HDC  hDC;
    int  xTime;
    HDWP hdwp;
    char szBuf[32];

    FormatTimeStr(szBuf);

    hDC = GetDC(hWnd);

    xTime = GetTextWidth(hDC, szBuf) + (xSysChar * 2) + 4;

    ReleaseDC(hWnd, hDC);

    hdwp = BeginDeferWindowPos(2);

    hdwp = DefMovSubWnd(hdwp, hWnd, WID_INFOTIME,
                        cxw - xTime + 2, 2, xTime - 4, cyw - 4);
    cxw -= xTime;

    hdwp = DefMovSubWnd(hdwp, hWnd, WID_INFOMSG, 2, 2, cxw - 4, cyw - 4);

    EndDeferWindowPos(hdwp);
}


/*----------------------------------------------------------------------------*/
VOID PRVFUNC DrawMsgTime
/*                                        */
/* Update the current time display.                       */
/*                                        */
/*----------------------------------------------------------------------------*/
(VOID)      /* No parameters.                             */
/*----------------------------------------------------------------------------*/

{
    HWND hWnd;

    char szBuf[32];
    static char szPrevBuf[32] = "";

    FormatTimeStr(szBuf);

    hWnd = hAppWnd;
    hWnd = hWnd ? GetDlgItem(hWnd, WID_INFO) : NULL;

    if ((strlen(szPrevBuf) != strlen(szBuf)) || (strncmp(szPrevBuf, szBuf, 3) != 0)) {
        RECT rc;

        GetClientRect(hWnd, &rc);
        UpdateWindowPos(hWnd, rc.right, rc.bottom);
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

        lstrcpy(szPrevBuf, szBuf);
    }

    hWnd = hWnd ? GetDlgItem(hWnd, WID_INFOTIME) : NULL;

    if (hWnd)
        SetText(hWnd, NULL, szBuf);
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC Msg_OnCommand
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
BOOL PRVFUNC Msg_OnCreate
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit windows creation stuff.                      */
/*                                        */
(HWND           hWnd,         /* Window that received the command.    */
 LPCREATESTRUCT X(lpCreateStruct)) /* Create structure.            */
/*----------------------------------------------------------------------------*/

{
    int idControl;

    for (idControl = WID_INFOMSG; idControl <= WID_INFOTIME; idControl++)
        CrtSubWnd(szClsKermText, NULL, hWnd, idControl, hAppInst);

    DrawMsgTime();
    SetMessage(0, NULL);

    return(TRUE);
}


/*----------------------------------------------------------------------------*/
void PRVFUNC Msg_OnSize
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle kermit window size change.                          */
/*                                        */
(HWND      hWnd,     /* Window that received the command.             */
 UINT      X(state), /* Window state.                         */
 int       cxw,      /* New window width.                     */
 int       cyw)      /* New window height.                    */
/*----------------------------------------------------------------------------*/

{
    UpdateWindowPos(hWnd, cxw, cyw);
}


/*---------------------------------------------------------------------------*/
LONG CALLBACK __export MsgWndProc
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

        HANDLE_MSG(hWnd, WM_COMMAND, Msg_OnCommand);

        HANDLE_MSG(hWnd, WM_CREATE, Msg_OnCreate);

        HANDLE_MSG(hWnd, WM_SIZE, Msg_OnSize);

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return(0L);
}


/*----------------------------------------------------------------------------*/
BOOL PUBFUNC MessageInit(HINSTANCE hInstance)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    WNDCLASS WndClass;

    memset(&WndClass, 0, sizeof(WndClass));

    WndClass.lpszClassName    = szClsKermMsg;
    WndClass.lpfnWndProc          = MsgWndProc;
    WndClass.style                = NULL;
    WndClass.hInstance        = hInstance;
//    WndClass.hbrBackground        = NULL;
    WndClass.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.hCursor              = LoadCursor(NULL, IDC_ARROW);

    return(RegisterClass(&WndClass) != NULL);
}


/*----------------------------------------------------------------------------*/
VOID PUBFUNC SetMessage
/*----------------------------------------------------------------------------*/
/*                                        */
/* Set the text in the message window.                        */
/*                                        */
(int   nMsgLevel,   /* Message level.                     */
 LPSTR lpszMsgText) /* Message text (void clears specified msg level).    */
/*----------------------------------------------------------------------------*/

{
    HWND hWnd;

    if (nMsgLevel < 0 || nMsgLevel > 4)
        return;

    if (lpszMsgText == NULL)
        szStatusMsg [nMsgLevel] [0] = '\0';
    else
        lstrcpy(szStatusMsg [nMsgLevel], lpszMsgText);


    hWnd = hAppWnd;
    hWnd = hWnd ? GetDlgItem(hWnd, WID_INFO) : NULL;
    hWnd = hWnd ? GetDlgItem(hWnd, WID_INFOMSG) : NULL;

    if (hWnd == NULL)
        return;

    for (nMsgLevel = 4; nMsgLevel >= 0; nMsgLevel--)
        if (szStatusMsg [nMsgLevel] [0] != '\0')
            break;

    if (nMsgLevel >= 0)
        SetText(hWnd, NULL, szStatusMsg [nMsgLevel]);
    else
        SetText(hWnd, NULL, "Ready");

    /* InvalidateRect(hWnd, NULL, TRUE); */
    UpdateWindow(hWnd);
}

/*----------------------------------------------------------------------------*/
void PUBFUNC MessageUpdate
/*----------------------------------------------------------------------------*/
/*                                        */
/* Handle timer interupts for message window.                        */
/*                                        */
(void)              /* No parms.                    */
/*----------------------------------------------------------------------------*/

{
    DrawMsgTime();
}
