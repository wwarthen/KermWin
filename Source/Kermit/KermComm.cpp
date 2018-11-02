/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMCOMM.C                                   **
**                                                                            **
**  This module contains the functions to interface with the Windows          **
**  interupt driven communications driver.                                    **
**                                                                            **
*******************************************************************************/

/* DEFINES -------------------------------------------------------------------*/

#define ASYNC   (nConnType == CT_ASYNC)
#define TCPIP   (nConnType == CT_TCPIP)

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include "kermasy.h"
#include "kermtcp.h"

/* CONSTANTS -----------------------------------------------------------------*/

/* LOCAL DATA ----------------------------------------------------------------*/

static char    szCurModule [64] = "";
int     nConnType = CT_ASYNC;

VOID PRVFUNC DrawStComm(VOID)
{
    if (ASYNC)
        AsyDrawStComm();
    else if (TCPIP)
        TcpDrawStComm();
}


BOOL PUBFUNC Disconnect(VOID)
{
    if (ASYNC)
        return AsyDisconnect();
    else if (TCPIP)
        return TcpDisconnect();

    return FALSE;
}

BOOL PUBFUNC Connect(VOID)
{
    if (ASYNC)
        return AsyConnect();
    else if (TCPIP)
        return TcpConnect();

    return FALSE;
}

VOID PUBFUNC PauseComm(BOOL bPause)
{
    if (ASYNC)
        AsyPauseComm(bPause);
}

VOID PUBFUNC DebugComm(VOID)
{
    if (ASYNC)
        AsyDebugCommMsgBox();
}

int PUBFUNC FlushCommQueue(int fnQueue)
{
    if (ASYNC)
        return AsyFlushCommQueue(fnQueue);

    return 0;
}

VOID PUBFUNC SendBreak(VOID)
{
    if (ASYNC)
        AsySendBreak();
}

VOID PUBFUNC SetCommDTRState(BOOL bAssert)
{
    if (ASYNC)
        AsySetCommDTRState(bAssert);
}

VOID PUBFUNC CheckCommStatus(UINT * pcbInQue, UINT * pcbOutQue)
{
    if (ASYNC)
        AsyCheckCommStatus(pcbInQue, pcbOutQue);
    else if (TCPIP)
        TcpCheckCommStatus(pcbInQue, pcbOutQue);
}

int PUBFUNC ReadCommStr(LPSTR lpsDest, int nMaxChars)
{
    if (ASYNC)
        return AsyReadCommStr(lpsDest, nMaxChars);
    else if (TCPIP)
        return TcpReadCommStr(lpsDest, nMaxChars);

    return 0;
}

int PUBFUNC WriteCommStr(LPSTR lpsCommStr, int nStrLen)
{
    if (ASYNC)
        return AsyWriteCommStr(lpsCommStr, nStrLen);
    else if (TCPIP)
        return TcpWriteCommStr(lpsCommStr, nStrLen);

    return 0;
}

/*----------------------------------------------------------------------------*/
int PUBFUNC WriteCommChar(char cCommChar)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    return(WriteCommStr(&cCommChar, 1));
}

/*----------------------------------------------------------------------------*/
int PUBFUNC WriteCommFmt(PSTR pszFmt, ...)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char        sWork[128];
    va_list     pArg;
    int         nLen;

    va_start(pArg, pszFmt);
    nLen = wvsprintf(sWork, pszFmt, pArg);
    va_end(pArg);

    return(WriteCommStr(sWork, nLen));
}

UINT PUBFUNC GetCommInfo(int nIndex)
{

    switch (nIndex) {
        case GCI_CONNTYPE:
            return(nConnType);
            break;

        default:
            if (ASYNC)
                return AsyGetCommInfo(nIndex);
            else if (TCPIP)
                return TcpGetCommInfo(nIndex);
    }

    return 0;
}

LRESULT CALLBACK __export CommNotifyWndProc(HWND hWnd, UINT message,
                                            WPARAM wParam, LPARAM lParam)
{
    if (ASYNC)
        return AsyCommNotifyWndProc(hWnd, message, wParam, lParam);
    else if (TCPIP)
        return TcpCommNotifyWndProc(hWnd, message, wParam, lParam);

    return 0L;
}

BOOL PUBFUNC CommInit(HINSTANCE hInstance)
{
    WNDCLASS    WndClass;

    memset(&WndClass, 0, sizeof(WndClass));

    WndClass.lpszClassName        = "KermCommNotify";
    WndClass.lpfnWndProc          = CommNotifyWndProc;
    WndClass.hInstance            = hInstance;

    if (RegisterClass(&WndClass) == NULL) {
        KermitFmtMsgBox(MB_OK, "Can't create Comm Notification Window!");
        return FALSE;
    }

    return(TRUE);
}

BOOL PUBFUNC CMInit(VOID)
{
    BOOL bResult;

    bConnected = FALSE;

    nConnType = CT_ASYNC;

    if (ASYNC)
        bResult = CMAsyInit();
    else if (TCPIP)
        bResult = CMTcpInit();
    else
        bResult = FALSE;

//    DrawStComm();

    return bResult;
}

BOOL PUBFUNC CMLoad(PSTR pszModule, BOOL bReset)
{
    BOOL bResult;

    if (!bReset && strcmp(szCurModule, pszModule) == 0)
        return TRUE;

    lstrcpy(szCurModule, pszModule);

    /* we can't load a new connection type while connected!!! */

    if (bConnected) {
        MessageBeep(0);
        return FALSE;
    }

    nConnType = CT_ASYNC;
    if (strcmp(pszModule, "tcpip") == 0)
        nConnType = CT_TCPIP;

    if (ASYNC)
        bResult = CMAsyLoad();
    else if (TCPIP)
        bResult = CMTcpLoad();
    else
        bResult = FALSE;

    DrawStComm();

    return bResult;
}

BOOL PUBFUNC CMSetConfig(UINT wInfoSize, PSTR pszInfo)
{
    BOOL bResult;

    if (ASYNC)
        bResult = CMAsySetConfig(wInfoSize, pszInfo);
    else if (TCPIP)
        bResult = CMTcpSetConfig(wInfoSize, pszInfo);
    else
        bResult = FALSE;

    DrawStComm();

    return bResult;
}

BOOL PUBFUNC CMGetConfig(UINT * pwInfoSize, PSTR pszInfo)
{
    BOOL bResult;

    if (ASYNC)
        bResult = CMAsyGetConfig(pwInfoSize, pszInfo);
    else if (TCPIP)
        bResult = CMTcpGetConfig(pwInfoSize, pszInfo);
    else
        bResult = FALSE;

    return bResult;
}

BOOL PUBFUNC CMSetup(HINSTANCE hInst, HWND hWnd)
{
    BOOL bResult;


    if (ASYNC)
        bResult = CMAsySetup(hInst, hWnd);
    else if (TCPIP)
        bResult = CMTcpSetup(hInst, hWnd);
    else
        bResult = FALSE;

    DrawStComm();

    return bResult;
}