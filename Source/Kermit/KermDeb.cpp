/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMDEB.C                                    **
**                                                                            **
**  This module contains the debugging routines.                              **
**                                                                            **
*******************************************************************************/

/* INCLUDES ------------------------------------------------------------------*/

#include "kermit.h"
#include <commdlg.h>
#include <time.h>
#ifndef _WIN32
#include <ver.h>
#endif

/* LOCAL DATA ----------------------------------------------------------------*/

#ifdef _WIN32
static CRITICAL_SECTION csDebug;
#endif

PATHNAM szIniFileName;

/*----------------------------------------------------------------------------*/
PSTR PUBFUNC DebugFmt(PSTR pszFmt, ...)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char szWork[1024];
    va_list     pArg;
    int         nLen;

    va_start(pArg, pszFmt);
    nLen = wvsprintf(szWork, pszFmt, pArg);
    va_end(pArg);

    return szWork;
}



/*----------------------------------------------------------------------------*/
VOID PUBFUNC WriteDebug(char * pszDebugMsg)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int         nDebugFile;

#ifdef _WIN32
    EnterCriticalSection(&csDebug);
#endif

    switch (nDebugDest) {
        case DD_MSGBOX:
            MessageBox(hAppWnd, pszDebugMsg, szAppName, MB_OK | MB_ICONASTERISK);
            break;

        case DD_MONITOR:
            OutputDebugString(pszDebugMsg);
            OutputDebugString("\r\n");
            break;

        case DD_FILE:
            nDebugFile = OpenFile(NULL, &ofDebugFile, OF_REOPEN | OF_WRITE);
            _llseek(nDebugFile, 0, SEEK_END);
            if (nDebugFile != -1) {
                _lwrite(nDebugFile, pszDebugMsg, lstrlen(pszDebugMsg));
                _lwrite(nDebugFile, "\r\n", 2);
                _lclose(nDebugFile);
            }
            break;
    }

#ifdef _WIN32
    LeaveCriticalSection(&csDebug);
#endif
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DebMsg(int nLevel, char * pszDebugMsg, ...)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    static char     szOutputLine[1024];
    va_list     pArg;

    if (nLevel > nDebugLevel)
        return;

#ifdef _WIN32
    EnterCriticalSection(&csDebug);
#endif

    _strdate_s(szOutputLine, sizeof(szOutputLine));
    lstrcat(szOutputLine, " ");
    _strtime_s(szOutputLine + lstrlen(szOutputLine), sizeof(szOutputLine) - lstrlen(szOutputLine));
    lstrcat(szOutputLine, " ");

    va_start(pArg, pszDebugMsg);
    wvsprintf(szOutputLine + lstrlen(szOutputLine), pszDebugMsg, pArg);
    va_end(pArg);

    WriteDebug(szOutputLine);

#ifdef _WIN32
    LeaveCriticalSection(&csDebug);
#endif
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DebugInit(HANDLE X(hInstance), HANDLE X(hPrevInstance),
                       LPSTR lpszCmdLine, int X(nCmdShow))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szDebugDest[80];
    int  nDebugFile;

    GetModuleFileName(hAppInst, szIniFileName, sizeof(szIniFileName));
    *(strrchr(szIniFileName, '\\') + 1) = '\0';
    lstrcat(szIniFileName, "Kermit.ini");

    nDebugLevel = DL_NONE;
    nDebugDest = DD_NONE;

    nDebugLevel = GetPrivateProfileInt(szAppName, "DebugLevel", 0, szIniFileName);

    if (nDebugLevel <= 0)
        return;

    GetPrivateProfileString(szAppName, "DebugDest", "", szDebugDest,
                     sizeof(szDebugDest), szIniFileName);

    if (lstrlen(szDebugDest) == 0) {
        MessageBox(hAppWnd, "No DebugDest specified!",
                   szAppName, MB_OK | MB_ICONASTERISK);
        return;
    }

    if (lstrcmpi(szDebugDest, "MsgBox") == 0)
        nDebugDest = DD_MSGBOX;
    else if (lstrcmpi(szDebugDest, "Monitor") == 0)
        nDebugDest = DD_MONITOR;
    else {
        nDebugFile = OpenFile(szDebugDest, &ofDebugFile, 0);
        if (nDebugFile == -1)
        {
            nDebugFile = OpenFile(szDebugDest, &ofDebugFile, OF_CREATE);
            if (nDebugFile == -1) {
                MessageBox(hAppWnd, "Can't open DebugDest file!",
                           szAppName, MB_OK | MB_ICONASTERISK);
                return;
            }
        }
        _lclose(nDebugFile);
        nDebugDest = DD_FILE;
    }

#ifdef _WIN32
    InitializeCriticalSection(&csDebug);
#endif

    if (nDebugDest == DD_MONITOR)
        WriteDebug("\r\n\r\n");

    DebMsg(1, "*** Kermit Starting - Debug Level %i ***", nDebugLevel);
    DebMsg(DL_INFO, "Command Line '%s'", (LPSTR)lpszCmdLine);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DebugTerm()

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (nDebugLevel <= 0)
        return;

    DebMsg(1, "*** Kermit Ending - Return Code 0 ***");

#ifdef _WIN32
    DeleteCriticalSection(&csDebug);
#endif
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DebugAddMenu(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    HMENU  hAppMenu;
    HMENU  hDebMenu;

    hAppMenu = GetMenu(hAppWnd);

    hDebMenu = LoadMenu(hAppInst, "DebugMenu");

    AppendMenu(hAppMenu, MF_POPUP | MF_STRING, (UINT)hDebMenu, "&Debug");
}

/*----------------------------------------------------------------------------*/
VOID PRVFUNC DebugShowVerInfo(VOID)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    int             i;
    BYTE            abInfoBuf[2048];
    DWORD           dwInfoHandle;
    DWORD           dwInfoSize;
    UINT            cbValueLength;
    LPVOID          lpszValue;
    char            szFileName[80];
    char            szValueName[80];
    char            szValueNamePrefix[80];
    OPENFILENAME    ofn;
    LONG FAR *      lplXlate;
    VS_FIXEDFILEINFO FAR * lpffi;
    static char *   pszValueNameList [] = {"CompanyName",
                                           "FileDescription",
                                           "FileVersion",
                                           "InternalName",
                                           "LegalCopyright",
                                           "LegalTrademarks",
                                           "OriginalFilename",
                                           "ProductName",
                                           "ProductVersion",
                                           NULL};

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hAppWnd;
    ofn.lpstrFilter = "Applicatons(*.EXE)\0*.exe\0"
                      "Dynamic Link Libraries(*.DLL)\0*.dll\0"
                      "Emulation Drivers(*.TRM)\0*.trm\0"
                      "All Files(*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "EXE";

    GetModuleFileName(hAppInst, szFileName, sizeof(szFileName));

    if (!GetOpenFileName(&ofn))
        return;

    dwInfoSize = GetFileVersionInfoSize(szFileName, &dwInfoHandle);

    if (dwInfoSize == 0) {
        KermitFmtMsgBox(MB_OK, "File %s has no version info!", (LPSTR)szFileName);
        return;
    }

    if (!GetFileVersionInfo(szFileName, dwInfoHandle, dwInfoSize, abInfoBuf)) {
        KermitFmtMsgBox(MB_OK, "Error getting version info from file %s!",
                        (LPSTR)szFileName);
        return;
    }

    if (!VerQueryValue(abInfoBuf, "\\", (VOID FAR * FAR *)&lpffi, &cbValueLength))
        KermitFmtMsgBox(MB_OK, "Can't get root version info!");

    WriteTermFmt("\r\n\r\n***** File: %s *****\r\n\r\n", (LPSTR)szFileName);

    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwSignature",        lpffi->dwSignature);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwStrucVersion",     lpffi->dwStrucVersion);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileVersionMS",    lpffi->dwFileVersionMS);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileVersionLS",    lpffi->dwFileVersionLS);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwProductVersionMS", lpffi->dwProductVersionMS);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwProductVersionLS", lpffi->dwProductVersionLS);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileFlagsMask",    lpffi->dwFileFlagsMask);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileFlags",        lpffi->dwFileFlags);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileOS",           lpffi->dwFileOS);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileType",         lpffi->dwFileType);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileSubtype",      lpffi->dwFileSubtype);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileDateMS",       lpffi->dwFileDateMS);
    WriteTermFmt("%24s = %.8lX\r\n", (LPSTR)"Root:dwFileDateLS",       lpffi->dwFileDateLS);

    if (!VerQueryValue(abInfoBuf, "\\VarFileInfo\\Translation",
                       (VOID FAR * FAR *)&lplXlate, &cbValueLength)) {
        KermitFmtMsgBox(MB_OK, "Can't get translation info '\\VarFileInfo\\Translation')!");
        return;
    }

    wsprintf(szValueNamePrefix, "\\StringFileInfo\\%.4X%.4X",
             LOWORD(*lplXlate), HIWORD(*lplXlate));

    for (i = 0; pszValueNameList[i] != NULL; i++) {
        wsprintf(szValueName, "%s\\%s", (LPSTR)szValueNamePrefix, (LPSTR)pszValueNameList[i]);

        if (!VerQueryValue(abInfoBuf, szValueName, &lpszValue, &cbValueLength))
            continue;

    WriteTermFmt("%24s = %s\r\n", (LPSTR)pszValueNameList[i], (LPSTR)lpszValue);
    }
}

/*----------------------------------------------------------------------------*/
BOOL PUBFUNC DebugMenuCmd(HWND X(hWnd), int Id)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    unsigned char c;

    switch (Id) {
        case IDM_DEBDCB:
            if (bConnected)
                DebugComm();
            else
                MessageBeep(0);
            break;

        case IDM_DEBTEST1:
            for (c = 0; c < 255; c++)
                WriteTermStr((PSTR)&c, 1, TRUE);
            break;

        case IDM_DEBTEST2:
            DebugShowVerInfo();
            break;

        case IDM_DEBTEST3:
            {
                int x;

                for (x = '1'; x <= '4'; x++) {
                    char c2[800] = "This is a test...";

                    memset(c2, x, sizeof(c2));
                    WriteCommStr(c2, sizeof(c2));
                }
            }
            break;

        case IDM_DEBTEST4:
            AXScriptLoadImmed(
                "Sub Hello()\n"
                "   kermit.write(\"Hello\")\n"
                "End Sub"
                );
            break;

        case IDM_DEBTEST5:
            AXScriptLoadImmed(
                "Call Hello()\n"
                "Call Hello()"
                );
            break;

        case IDM_DEBTEST6:
            AXScriptExec("Hello");
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DebAssert(char *filename, int linenum, int exp, char *expstr)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    if (exp)
        return;

    DebMsg(DL_ERROR, "ASSERTION FAILED - File: %s, Line: %d - %s",
               (LPSTR)filename, linenum, (LPSTR)expstr);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DebTrace(char *filename, int linenum, int nLevel, PSTR pszFmt, ...)

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

    DebMsg(nLevel, "TRACE - File: %s, Line: %d - %s",
               (LPSTR)filename, linenum, (LPSTR)sWork);
}

/*----------------------------------------------------------------------------*/
VOID PUBFUNC DebDump(int nLevel, void far *data, unsigned long length, char *desc, ...)

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szWorkStr[128];
    unsigned long i, j;
    va_list     pArg;

    if (nLevel > nDebugLevel)
        return;

#ifdef _WIN32
    EnterCriticalSection(&csDebug);
#endif

    va_start(pArg, desc);
    wvsprintf(szWorkStr, desc, pArg);
    va_end(pArg);

    DebMsg(nLevel, szWorkStr);

    for (i = 0; i < length; i+=0x10)
    {
        wsprintf(szWorkStr, "  %0.4lX: ", i);

        for (j = 0; j < 0x10; j++)
        {
            static char map[] = "0123456789ABCDEF";

            if (i + j >= length)
                lstrcat(szWorkStr, "  ");
            else
            {
                int nDumpStr;

                nDumpStr = lstrlen(szWorkStr);

                szWorkStr[nDumpStr++] = map[((((char far *)data)[i + j]) >> 4) & 0xF];
                szWorkStr[nDumpStr++] = map[(((char far *)data)[i + j]) & 0xF];

                szWorkStr[nDumpStr] = '\0';
            }

            lstrcat(szWorkStr, " ");

            if (j == 7)
                lstrcat(szWorkStr, "- ");
        }

        lstrcat(szWorkStr, "  *");

        for (j = 0; j < 0x10; j++)
        {
            int nDumpStr;

            if (i + j >= length)
                break;

            nDumpStr = lstrlen(szWorkStr);

            if (((char far *)data)[i + j] < ' ')
                szWorkStr[nDumpStr++] = '?';
            else
                szWorkStr[nDumpStr++] = ((char far *)data)[i + j];

            szWorkStr[nDumpStr] = '\0';
        }

        lstrcat(szWorkStr, "*");

        WriteDebug(szWorkStr);
    }

#ifdef _WIN32
    LeaveCriticalSection(&csDebug);
#endif
}
