/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMMISC.C                                   **
**                                                                            **
**  This module contains miscellaneous routines for the applications.         **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"

/*----------------------------------------------------------------------------*/
int PUBFUNC GoDialogBox(HINSTANCE hInstance, LPSTR lpTemplateName,
            HWND hWndParent, FARPROC lpProc)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    int     nResult;
    FARPROC lpDialog;

    lpDialog = MakeProcInstance(lpProc, hInstance);
    nResult = DialogBox(hInstance, lpTemplateName, hWndParent, (DLGPROC)lpDialog);
    X(FreeProcInstance(lpDialog);)

    return(nResult);
}

/*----------------------------------------------------------------------------*/
HFILE PUBFUNC ErrOpenFile(LPSTR lpFileName, LPOFSTRUCT lpReOpenBuff, UINT wStyle)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    HFILE hResult;

    SetErrorMode(1);
    hResult = OpenFile(lpFileName, lpReOpenBuff, wStyle);
    SetErrorMode(0);

    return(hResult);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC ErrDlgDirList(HWND hDlg, LPSTR lpPathSpec, int nIDListBox,
              int nIDStaticPath, unsigned wFileType)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    int nResult;

    SetErrorMode(1);
    nResult = DlgDirList(hDlg, lpPathSpec, nIDListBox,
                         nIDStaticPath, wFileType);
    SetErrorMode(0);

    return(nResult);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC ErrDlgDirListComboBox(HWND hDlg, LPSTR lpPathSpec, int nIDListBox,
              int nIDStaticPath, unsigned wFileType)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    int nResult;

    SetErrorMode(1);
    nResult = DlgDirListComboBox(hDlg, lpPathSpec, nIDListBox,
                                 nIDStaticPath, wFileType);
    SetErrorMode(0);

    return(nResult);
}

/*----------------------------------------------------------------------------*/
HWND PUBFUNC CrtSubWnd (LPCSTR lpszClassName, DWORD dwStyle,
            HWND hWndParent, int idCtl, HINSTANCE hInst)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    return(CreateWindow(lpszClassName, NULL, WS_CHILDWINDOW | WS_CLIPSIBLINGS |
//            WS_CLIPCHILDREN | WS_BORDER | WS_VISIBLE | dwStyle,
            WS_CLIPCHILDREN | WS_VISIBLE | dwStyle,
            0, 0, 0, 0, hWndParent, (HMENU)idCtl, hInst, NULL));
}

/*----------------------------------------------------------------------------*/
HDWP PUBFUNC DefMovSubWnd(HDWP hdwp, HWND hWndParent, int idCtl,
              int x, int y, int cx, int cy)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    HWND hChildWnd;

    if ((hChildWnd = GetDlgItem(hWndParent, idCtl)) == NULL)
    return(hdwp);

    return(DeferWindowPos(hdwp, hChildWnd, HWND_BOTTOM,
              x, y, cx, cy,
              SWP_DRAWFRAME | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW));
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC MoveChildWindow(HWND hWnd, int nIDChild, int X, int Y,
                 int nWidth, int nHeight, BOOL X(bRepaint))

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    HWND hChildWnd;
    static HDWP hdwp;

    if (hWnd == 0) {
    hdwp = BeginDeferWindowPos(5);
        return;
    }

    if (hWnd == (HWND)1) {
    EndDeferWindowPos(hdwp);
        return;
    }

    if ((hChildWnd = GetDlgItem(hWnd, nIDChild)) == NULL)
    return;

    hdwp = DeferWindowPos(hdwp, hChildWnd, HWND_BOTTOM, X, Y,
              nWidth, nHeight, SWP_DRAWFRAME);

    /* MoveWindow(hChildWnd, X, Y, nWidth, nHeight, bRepaint); */
}

/*----------------------------------------------------------------------------*/
HDWP PUBFUNC DeferChildWindowPos(HDWP hdwp, HWND hWndParent, int idCtl,
                 HWND hWndAfter, int x, int y,
                 int cx, int cy, UINT flags)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    HWND hChildWnd;

    if ((hChildWnd = GetDlgItem(hWndParent, idCtl)) == NULL)
    return(hdwp);

    return(DeferWindowPos(hdwp, hChildWnd, hWndAfter,
              x, y, cx, cy, flags));
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC SetKermitMenu(LPSTR lpMenuName)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    ModifyMenu(GetMenu(hAppWnd), 3, MF_POPUP | MF_BYPOSITION,
               (UINT)LoadMenu(hAppInst, lpMenuName), "&Kermit");

    DrawMenuBar(hAppWnd);
}

/*----------------------------------------------------------------------------*/
int PUBFUNC KermitFmtMsgBox(UINT wType, PSTR pszFmt, ...)

/*  Description of what function does.                        */

/*  Param(s):   x...............Description.                      */

/*  Returns:    Result description.                       */

{
    char        sWork[1024];
    va_list     pArg;
    int         nLen;

    va_start(pArg, pszFmt);
    nLen = wvsprintf(sWork, pszFmt, pArg);
    va_end(pArg);

    return(MessageBox(hAppWnd, sWork, szAppName, wType));
}


#if 0
/*----------------------------------------------------------------------------*/
VOID PUBFUNC AppBusy(BOOL bBusy)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    POINT pt;

    if (bBusy) {
        if (nAppBusy == 0) {
            EnableWindow(hAppWnd, FALSE);
            GetCursorPos(&pt);
            SetCursorPos((int)pt.x, (int)pt.y);
        }

        nAppBusy++;
    }
    else {
        if (nAppBusy == 0)
            return;

        nAppBusy--;

        if (nAppBusy == 0) {
            GetCursorPos(&pt);
            SetCursorPos((int)pt.x, (int)pt.y);
            EnableWindow(hAppWnd, TRUE);
        }
    }
}

#endif

PSTR PUBFUNC StrCatDefExt(PSTR pszFileName, PSTR pszDefExt)
{
    PSTR pszFileNameIdx;

    if (pszFileName == NULL)
        return NULL;

    pszFileNameIdx = strrchr(pszFileName, '\\');

    if (pszFileNameIdx == NULL)
        pszFileNameIdx = pszFileName;
    else
        pszFileNameIdx++;

    if (!strchr(pszFileNameIdx, '.'))
        lstrcat(pszFileName, pszDefExt);

    return pszFileName;
}

DWORD PUBFUNC SysErrText(DWORD dwErrCode, LPSTR lpszMsgBuf, DWORD dwMsgBufSize)
{
    DWORD       dwLen;

    if (lpszMsgBuf == NULL)
        return 0;

    lpszMsgBuf[0] = '\0';

    dwLen = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL,
        dwErrCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        lpszMsgBuf,
        dwMsgBufSize,
        NULL);

    return dwLen;
}

BOOL PUBFUNC GetFileVerString(PSTR szFileName, PSTR szVerString)
{
    BYTE            abInfoBuf[2048];
    DWORD           dwInfoHandle;
    DWORD           dwInfoSize;
    UINT            cbValueLength;
    LPVOID          lpszProductName;
    LPVOID          lpszProductVersion;
    char            szValueName[80];
    char            szValueNamePrefix[80];
    LONG FAR *      lplXlate;
    VS_FIXEDFILEINFO FAR * lpffi;

    dwInfoSize = GetFileVersionInfoSize(szFileName, &dwInfoHandle);

    if (dwInfoSize == 0)
        return FALSE;

    if (!GetFileVersionInfo(szFileName, dwInfoHandle, dwInfoSize, abInfoBuf))
        return FALSE;

    if (!VerQueryValue(abInfoBuf, "\\", (VOID FAR * FAR *)&lpffi, &cbValueLength))
        return FALSE;

    if (!VerQueryValue(abInfoBuf, "\\VarFileInfo\\Translation",
                       (VOID FAR * FAR *)&lplXlate, &cbValueLength))
        return FALSE;

    wsprintf(szValueNamePrefix, "\\StringFileInfo\\%.4X%.4X",
             LOWORD(*lplXlate), HIWORD(*lplXlate));

    wsprintf(szValueName, "%s\\%s", (LPSTR)szValueNamePrefix, (LPSTR)"ProductName");
    if (!VerQueryValue(abInfoBuf, szValueName, &lpszProductName, &cbValueLength))
        lpszProductName = "?";

    wsprintf(szValueName, "%s\\%s", (LPSTR)szValueNamePrefix, (LPSTR)"ProductVersion");
    if (!VerQueryValue(abInfoBuf, szValueName, &lpszProductVersion, &cbValueLength))
        lpszProductVersion = "?";

    wsprintf(szVerString, "%s Version %s", (LPSTR)lpszProductName, (LPSTR)lpszProductVersion);

    return TRUE;
}