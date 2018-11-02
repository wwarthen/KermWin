/*******************************************************************************
**                                                                            **
**                      Setup for Microsoft Windows                           **
**                      ----------------------------                          **
**                                SETUP.C                                     **
**                                                                            **
**                      Version 0.50 - Developmental                          **
**                     Requires Microsoft Windows 2.X                         **
**                         Author: Wayne Warthen                              **
**                                                                            **
**  Primary application source file containing the WinMain() and the client   **
**  Window message handling procedure and support functions.                  **
**                                                                            **
*******************************************************************************/

/* TO DO ---------------------------------------------------------------------

  1. Need more error checking in ProcessData?()'s.
  2. Errors in ProcessData?()'s are ignored.

   DEFINITIONS ---------------------------------------------------------------*/

/* INCLUDES ------------------------------------------------------------------*/

#include "setup.h"
#include "setupres.h"
#ifndef _WIN32
#include <ver.h>
#include <ddeml.h>
#include <shellapi.h>
#endif
#include <direct.h>
#include <process.h>
#include <errno.h>
#include "..\Kermit\KermC3D.h"

#define SR_OK       0
#define SR_USERCAN  1
#define SR_WARNING  2
#define SR_FAILED   3

#define MODE_INTERACTIVE    1
#define MODE_BATCH          2
#define MODE_STEALTH        3

static PATHNAM szDirDlgDirectory;
static PATHNAM szRunAtExitCmdLine;
static char    szDirDlgPrompt [80];
static BOOL bAbort = FALSE;
static int nResultSetup = SR_OK;
static int nMode = MODE_INTERACTIVE;
static char szBuf[1024];
static BOOL bMainProcessed = FALSE;

static char * szAppName = "Setup";
static char   szVersion[] = "Version " VER_DESC " (" BUILD_OS ") " BUILD_TYPE;
static char   szBuild[] = REL_TYPE " Build " BLD_DESC ", " BUILD_DATE;
static char   szCopyright[] = COPYRIGHT;
static char   szContact[] = CONTACT;

static HINSTANCE hAppInst = NULL;          /* instance handle                         */
static HWND      hAppWnd = NULL;           /* handle to the main window               */

static FARPROC lpProgressDlgProc = NULL;
static HWND    hProgressDlg = NULL;

static HCONV hConv = NULL;
static DWORD idInst = 0;
static FARPROC lpDdeProc = NULL;


#ifdef _WIN32
#define W16W32(x, y) y
#else
#define W16W32(x, y) x
#endif

#ifdef _WIN32
#define ERRNUMTYPE DWORD
#else
#define ERRNUMTYPE int
#endif

char * StrOSErr(ERRNUMTYPE ErrNum)
{
    static char szErrMsgBuf[80];

#ifdef _WIN32
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            ErrNum,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)szErrMsgBuf,
            sizeof(szErrMsgBuf),
            NULL
        );
#else
        lstrcpy(szErrMsgBuf, strerror(ErrNum));
#endif

    return szErrMsgBuf;
}

/* Variables below are visible to command files via VARMAP!!! */

static char    szAppTitle[80];
static PATHNAM szSrcDir;
static PATHNAM szDestDir;
static PATHNAM szSysDir;
static PATHNAM szTempDir;
static PATHNAM szSetupCmdFile;
static char    szProcessID[20];

typedef struct
{
    char * pszVarName;
    char * pszVariable;
} VARMAP;

VARMAP VarMapTable[] =
{
    {"AppTitle",    szAppTitle},
    {"SrcDir",      szSrcDir},
    {"DestDir",     szDestDir},
    {"SysDir",      szSysDir},
    {"TempDir",     szTempDir},
    {"ProcessID",   szProcessID},
    {NULL, NULL}
};

char * GetVar(char * pszVarName)
{
    int i;

    for (i = 0; VarMapTable[i].pszVarName != NULL; i++)
    {
        if (lstrcmpi(pszVarName, VarMapTable[i].pszVarName) == 0)
            return VarMapTable[i].pszVariable;
    }

    return NULL;
}

#define SetVar(pszVarName, pszValue) lstrcpy(GetVar(pszVarName), pszValue)

BOOL HandleMessages(VOID)
{
    MSG Msg;
    static BOOL bQuit = FALSE;

    if (bQuit)
        return FALSE;

    while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
        if (Msg.message == WM_QUIT) {
            bQuit = TRUE;
            break;
        }

        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return !bQuit;
}

int PUBFUNC GoDialogBox(LPCSTR lpTemplateName, HWND hWndParent, FARPROC lpProc)
{
    int     nResult;
    FARPROC lpDialog;

    lpDialog = MakeProcInstance(lpProc, hAppInst);
    nResult = DialogBox(hAppInst, lpTemplateName, hWndParent, (DLGPROC)lpDialog);
    X32(FreeProcInstance(lpDialog);)

    return(nResult);
}

BOOL ConfirmAbort(HWND X(hWnd))
{
    if (MessageBox(NULL, "Abort Setup?", szAppTitle,
             MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL) == IDYES)
    {
        bAbort = TRUE;
        nResultSetup = SR_USERCAN;
    }

    return bAbort;
}

int PUBFUNC FmtMsg(UINT wType, PSTR pszFmt, ...)
{
    char        sWork[1024];
    va_list     pArg;
    int         nLen;

    va_start(pArg, pszFmt);
    nLen = wvsprintf(sWork, pszFmt, pArg);
    va_end(pArg);

    return(MessageBox(hProgressDlg == NULL ? hAppWnd : hProgressDlg,
                      sWork, szAppTitle[0] == '\0' ? szAppName : szAppTitle, wType));
}

BOOL SetDlgItemFmtText(HWND hDlg, int nIDDlgItem, PSTR pszFmt, ...)
{
    char        sWork[1024];
    va_list     pArg;
    int         nLen;
    BOOL        bResult;

    va_start(pArg, pszFmt);
    nLen = wvsprintf(sWork, pszFmt, pArg);
    va_end(pArg);

#ifdef _WIN32
    bResult =
#else
    bResult = TRUE;
#endif
    SetDlgItemText(hDlg, nIDDlgItem, sWork);

    return bResult;
}

#ifdef _WIN32

BOOL ReplaceFileOnReboot (LPCTSTR pszExisting, LPCTSTR pszNew)
{
   // First, attempt to use the MoveFileEx function.
   BOOL fOk = MoveFileEx(pszExisting, pszNew, MOVEFILE_DELAY_UNTIL_REBOOT);
   if (fOk) return(fOk);

   // If MoveFileEx failed, we are running on Windows 95 and need to add
   // entries to the WININIT.INI file (an ANSI file).
   // Start a new scope for local variables.
   {
   char szRenameLine[1024];

   int cchRenameLine = wsprintfA(szRenameLine,
#ifdef UNICODE
      "%ls=%ls\r\n",
#else
      "%hs=%hs\r\n",
#endif
      (pszNew == NULL) ? __TEXT("NUL") : pszNew, pszExisting);
      char szRenameSec[] = "[Rename]\r\n";
      int cchRenameSec = sizeof(szRenameSec) - 1;
      HANDLE hfile, hfilemap;
      DWORD dwFileSize, dwRenameLinePos;
      TCHAR szPathnameWinInit[_MAX_PATH];

      // Construct the full pathname of the WININIT.INI file.
      GetWindowsDirectory(szPathnameWinInit, _MAX_PATH);
      lstrcat(szPathnameWinInit, __TEXT("\\WinInit.Ini"));

      // Open/Create the WININIT.INI file.
      hfile = CreateFile(szPathnameWinInit,
         GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

      if (hfile == INVALID_HANDLE_VALUE)
         return(fOk);

      // Create a file mapping object that is the current size of
      // the WININIT.INI file plus the length of the additional string
      // that we're about to insert into it plus the length of the section
      // header (which we might have to add).
      dwFileSize = GetFileSize(hfile, NULL);
      hfilemap = CreateFileMapping(hfile, NULL, PAGE_READWRITE, 0,
         dwFileSize + cchRenameLine + cchRenameSec, NULL);

      if (hfilemap != NULL) {

         // Map the WININIT.INI file into memory.  Note: The contents
         // of WININIT.INI are always ANSI; never Unicode.
         LPSTR pszWinInit = (LPSTR) MapViewOfFile(hfilemap,
            FILE_MAP_WRITE, 0, 0, 0);

         if (pszWinInit != NULL) {

            // Search for the [Rename] section in the file.
            LPSTR pszRenameSecInFile = strstr(pszWinInit, szRenameSec);

            if (pszRenameSecInFile == NULL) {

               // There is no [Rename] section in the WININIT.INI file.
               // We must add the section too.
               dwFileSize += wsprintfA(&pszWinInit[dwFileSize], "%s",
                                       szRenameSec);
               dwRenameLinePos = dwFileSize;

            } else {

               // We found the [Rename] section, shift all the lines down
               PSTR pszFirstRenameLine = strchr(pszRenameSecInFile, '\n');
               // Shift the contents of the file down to make room for
               // the newly added line.  The new line is always added
               // to the top of the list.
               pszFirstRenameLine++;   // 1st char on the next line
               memmove(pszFirstRenameLine + cchRenameLine, pszFirstRenameLine,
                  pszWinInit + dwFileSize - pszFirstRenameLine);
               dwRenameLinePos = pszFirstRenameLine - pszWinInit;
            }

            // Insert the new line
            memcpy(&pszWinInit[dwRenameLinePos], szRenameLine, cchRenameLine);

            UnmapViewOfFile(pszWinInit);

            // Calculate the true, new size of the file.
            dwFileSize += cchRenameLine;

            // Everything was successful.
            fOk = TRUE;
         }
         CloseHandle(hfilemap);
      }

      // Force the end of the file to be the calculated, new size.
      SetFilePointer(hfile, dwFileSize, NULL, FILE_BEGIN);
      SetEndOfFile(hfile);

      CloseHandle(hfile);
   }

   return(fOk);
}

#else

BOOL ReplaceFileOnReboot (LPSTR X(pszExisting), LPSTR X(pszNew))
{
    return TRUE;
}

#endif

/*----------------------------------------------------------------------------*/
BOOL CALLBACK __export AboutDlgProc(HWND hDlg, UINT message,
                    WPARAM wParam, LPARAM X(lParam))

/*  Description of what function does.                                        */

/*  Param(s):   x...............Description.                                  */

/*  Returns:    Result description.                                           */

{
    char szWork[512];

    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDD_OK:
            EndDialog(hDlg, TRUE);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        wsprintf(szWork, "%s\n\n%s\n%s", (LPSTR)szAppTitle,
                 (LPSTR)szVersion, (LPSTR)szBuild);
        SetDlgItemText(hDlg, IDD_VERSION, szWork);
        wsprintf(szWork, "%s\n\n%s", (LPSTR)szCopyright,
                 (LPSTR)szContact);
        SetDlgItemText(hDlg, IDD_MESSAGE, szWork);
        return(TRUE);

    default:
        return(FALSE);
    }
    return(TRUE);
}

BOOL CALLBACK __export DirDlgProc(HWND hDlg, UINT message,
                                  WPARAM wParam, LPARAM X(lParam))
{
    char szDirectory[256];

    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hDlg, IDC_DIR, szDirectory, sizeof(szDirectory));
            if (_chdir(szDirectory) == 0) {
                lstrcpy(szDirDlgDirectory, szDirectory);
                EndDialog(hDlg, TRUE);
            }
            else
            {
                wsprintf(szBuf, "Directory '%s' does not exist.\n\nCreate it?",
                    (LPSTR)szDirectory);
                switch (MessageBox(hDlg, szBuf, szAppTitle,
                    MB_ICONQUESTION | MB_YESNOCANCEL))
                {
                case IDYES:
                    if (_mkdir(szDirectory) == 0)
                    {
                        lstrcpy(szDirDlgDirectory, szDirectory);
                        EndDialog(hDlg, TRUE);
                    }
                    else
                        MessageBox(hDlg, "Can not create directory!",
                        szAppTitle, MB_ICONEXCLAMATION | MB_OK);
                    break;

                case IDCANCEL:
                    if (ConfirmAbort(hDlg))
                        EndDialog(hDlg, FALSE);
                    break;
                }
            }
            break;

        case IDCANCEL:
            if (ConfirmAbort(hDlg))
                EndDialog(hDlg, FALSE);
            break;

        default:
            return(FALSE);
        }
        break;

        case WM_INITDIALOG:
            SetDlgItemText(hDlg, IDC_DIR, szDirDlgDirectory);
            SetDlgItemText(hDlg, IDC_PROMPT, szDirDlgPrompt);
            return(TRUE);

        default:
            return(FALSE);
    }

    return(TRUE);
}

BOOL CALLBACK __export ProgressDlgProc(HWND hDlg, UINT message,
                                       WPARAM wParam, LPARAM X(lParam))
{
    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            if (ConfirmAbort(hDlg))
                EndDialog(hDlg, TRUE);
            break;

        default:
            return(FALSE);
        }

        break;

        case WM_INITDIALOG:
            return(TRUE);

        default:
            return(FALSE);
    }

    return(TRUE);
}

BOOL StartProgressDlg()
{
    /* Create and display progress dialog box */

    lpProgressDlgProc = MakeProcInstance((FARPROC)ProgressDlgProc, hAppInst);

    if (lpProgressDlgProc != NULL)
    {
        hProgressDlg = CreateDialog(hAppInst, "ProgressDlgBox", hAppWnd,
                                    (DLGPROC)lpProgressDlgProc);

        if (hProgressDlg != NULL)
        {
            ShowWindow(hProgressDlg, SW_HIDE);

            SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "Initializing...");
            SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");
        }
        else
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

BOOL EndProgressDlg()
{
    if (hProgressDlg != NULL)
    {
        DestroyWindow(hProgressDlg);
        hProgressDlg = NULL;
    }

    if (lpProgressDlgProc != NULL)
    {
        X32(FreeProcInstance(lpProgressDlgProc);)
        lpProgressDlgProc = NULL;
    }

    return TRUE;
}

VOID PRVFUNC Setup_OnCommand
(HWND   hWnd,
 int    Id,
 HWND   X(hWndCtl),
 UINT   X(wNotify))
{
    switch (Id)
    {
    case IDM_ABOUT:
        GoDialogBox("AboutDlgBox", hWnd, (FARPROC)AboutDlgProc);
        break;
    }
}

VOID Setup_OnSysCommand(HWND hWnd, UINT Cmd, int x, int y)
{
    switch (Cmd)
    {
    case IDM_ABOUT:
        GoDialogBox("AboutDlgBox", hWnd, (FARPROC)AboutDlgProc);
        break;

    default:
        FORWARD_WM_SYSCOMMAND(hWnd, Cmd, x, y, DefWindowProc);
        break;
    }
}

void PRVFUNC Setup_OnClose(HWND hWnd)
{
    ConfirmAbort(hWnd);
}

BOOL PRVFUNC Setup_OnQueryEndSession(HWND X(hWnd))
{
    return TRUE;
}

void PRVFUNC Setup_OnDestroy(HWND X(hWnd))
{
    C3DUnregister(hAppInst);

    PostQuitMessage(0);
}

VOID Setup_OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    RECT rc;
    HDC hDC;
    int i;
    DWORD yCur, yLast;
    HPEN hPen, hPrevPen;
    LOGFONT lf;
    HFONT hFont, hPrevFont;

    hDC = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &rc);

    yLast = 0;
    for (i = 0; i <= 255; i++) {
        yCur = (((DWORD)i * rc.bottom) / 255L) + 1;
        if (yCur == yLast)
            continue;

        hPen = CreatePen(PS_INSIDEFRAME, (int)((yCur - yLast) + 1), RGB(0, 0, 255 - i));
        hPrevPen = (HPEN)SelectObject(hDC, hPen);

        MoveToEx(hDC, 0, (int)yLast, NULL);
        LineTo(hDC, (int)rc.right, (int)yLast);
        yLast = yCur;

        SelectObject(hDC, hPrevPen);
        DeleteObject(hPen);
    }

    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = -MulDiv(36, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    lf.lfItalic = TRUE;
    lf.lfWeight = FW_BOLD;
    lstrcpy(lf.lfFaceName, "Arial");
    hFont = CreateFontIndirect(&lf);

    hPrevFont = (HFONT)SelectObject(hDC, hFont);
    SetBkMode(hDC, TRANSPARENT);
    TextOut(hDC, 36, 36, szAppTitle, lstrlen(szAppTitle));
    SetTextColor(hDC, RGB(255, 255, 255));
    TextOut(hDC, 32, 32, szAppTitle, lstrlen(szAppTitle));

    SelectObject(hDC, hPrevFont);
    DeleteObject(hFont);

    EndPaint(hWnd, &ps);
}

void PRVFUNC Setup_OnSysColorChange(HWND X(hWnd))
{
    C3DColorChange();
}

LONG CALLBACK __export SetupWndProc
(HWND     hWnd,
 UINT     message,
 WPARAM   wParam,
 LPARAM   lParam)
{
    switch (message) {

    HANDLE_MSG(hWnd, WM_COMMAND, Setup_OnCommand);

    HANDLE_MSG(hWnd, WM_SYSCOMMAND, Setup_OnSysCommand);

    HANDLE_MSG(hWnd, WM_CLOSE, Setup_OnClose);

    HANDLE_MSG(hWnd, WM_QUERYENDSESSION, Setup_OnQueryEndSession);

    HANDLE_MSG(hWnd, WM_DESTROY, Setup_OnDestroy);

    HANDLE_MSG(hWnd, WM_PAINT, Setup_OnPaint);

    HANDLE_MSG(hWnd, WM_SYSCOLORCHANGE, Setup_OnSysColorChange);

    default:
        return(DefWindowProc(hWnd, message, wParam, lParam));
    }
}

BOOL PUBFUNC InitSetupClass(void)
{
    WNDCLASS WndClass;

    memset(&WndClass, 0, sizeof(WndClass));

    WndClass.lpszClassName        = szAppName;
    WndClass.lpszMenuName         = NULL;
    WndClass.hInstance            = hAppInst;
    WndClass.lpfnWndProc          = SetupWndProc;
    WndClass.style                = CS_HREDRAW | CS_VREDRAW;
    WndClass.hCursor              = LoadCursor(NULL, IDC_ARROW);
    WndClass.hIcon                = LoadIcon(hAppInst, "SetupIcon");

//    return !(RegisterClass(&WndClass) == 0);
    RegisterClass(&WndClass);

    return TRUE;
}

BOOL PUBFUNC AppInit(HINSTANCE hInstance, HINSTANCE X(hPrevInstance),
        LPSTR lpszCmdLine, int X(cmdShow))
{
    HMENU hSysMenu;
    int i, j;
    char szWaitProcessID [20];

#ifdef _WIN32

    _itoa_s(GetCurrentProcessId(), szProcessID, sizeof(szProcessID), 10);

#endif

    szWaitProcessID[0] = '\0';

    if (lpszCmdLine != NULL)
    {
        for (i = j = 0; lpszCmdLine[i] != '\0' && lpszCmdLine[i] != ' '; i++)
            szSetupCmdFile[j++] = lpszCmdLine[i];
        szSetupCmdFile[j] = '\0';

        if (lpszCmdLine[i] == ' ')
        {
            for (i++, j = 0; lpszCmdLine[i] != '\0'; i++)
                szWaitProcessID[j++] = lpszCmdLine[i];
            szWaitProcessID[j] = '\0';
        }
    }

#ifdef _WIN32

    if (lstrlen(szWaitProcessID) > 0)
    {
        DWORD dwResult;
        HANDLE hWaitProcess;
        int i2;

        hWaitProcess = OpenProcess(SYNCHRONIZE, FALSE, atoi(szWaitProcessID));

        /* Caller will not return from WinExec unless callee (us) allow
           message loop to run (not entirely sure why).  It appears that
           sometimes it is necessary to run the message loop a couple times
           to ensure WinExec gets a change to return and allow calling app
           to run to completion.  The following loop will try the message
           loop up to 10 times waiting for the caller to terminate. */

        for (dwResult = WAIT_TIMEOUT, i2 = 0; (dwResult == WAIT_TIMEOUT) && (i2 < 10); i2++)
        {
            HandleMessages();
            dwResult = WaitForSingleObject(hWaitProcess, 1000);
        }

        switch (dwResult)
        {
        case WAIT_OBJECT_0:
//          FmtMsg(MB_OK, "Wait for completion succeeded!");
            break;

        case WAIT_TIMEOUT:
            FmtMsg(MB_OK, "Wait for completion timeout!");
            break;

        default:
            FmtMsg(MB_OK, "Wait for completion failed!");
            break;
        }
        CloseHandle(hWaitProcess);
    }

#endif

    hAppInst = hInstance;

    if (!(InitSetupClass()))
        return FALSE;

    hAppWnd = CreateWindow(szAppName, szAppName,
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           NULL, NULL, hAppInst, NULL);

    if (hAppWnd == NULL)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Can't create application window!\n\n"
               "Setup can't continue");
        return FALSE;
    }

    hSysMenu = GetSystemMenu(hAppWnd, FALSE);
    AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hSysMenu, MF_STRING, IDM_ABOUT, "&About...");

    C3DInit();

    C3DRegister(hInstance);
    C3DAutoSubclass(hInstance);

//    ShowWindow(hAppWnd, X(cmdShow) SW_SHOWMAXIMIZED);
//    ShowWindow(hAppWnd, X(cmdShow) SW_HIDE);
//    UpdateWindow(hAppWnd);

    if (!StartProgressDlg())
    {
        FmtMsg(MB_ICONHAND | MB_OK,
               "Error creating/displaying progress dialog!\n\n"
               "Setup can't continue.");
        return FALSE;
    }

    /* Establish source directory, presumed to be where program runs from! */

    GetModuleFileName(hInstance, szSrcDir, sizeof(szSrcDir));
    for (i = lstrlen(szSrcDir) - 1; i > 0; i--) {
        if (szSrcDir[i] == '\\') {
            szSrcDir[i] = '\0';
            break;
        }
    }

#ifdef _WIN32

    GetTempPath(sizeof(szTempDir), szTempDir);

#else

    GetTempFileName(0, "XXX", 0, szTempDir);
    for (i = lstrlen(szTempDir) - 1; i > 0; i--) {
        if (szTempDir[i] == '\\') {
            szTempDir[i] = '\0';
            break;
        }
    }

#endif

    if (szTempDir[lstrlen(szTempDir) - 1] == '\\')
        szTempDir[lstrlen(szTempDir) - 1] = '\0';

    /* Establish windows system directory */

    GetSystemDirectory(szSysDir, sizeof(szSysDir));

    return TRUE;
}

BOOL AppTerm(void)
{
    EndProgressDlg();

    if (!bAbort && lstrlen(szRunAtExitCmdLine) > 0)
    {
        WinExec(szRunAtExitCmdLine, SW_SHOWNORMAL);
//      FmtMsg(MB_OK, "RunAtExit issued '%s'", (LPSTR)szRunAtExitCmdLine);
    }

    if (hAppWnd != NULL)
        DestroyWindow(hAppWnd);

//    while (HandleMessages());
    if (HandleMessages())
    {
        FmtMsg(MB_OK | MB_ICONSTOP, "Setup is exitting prior to receipt of WM_QUIT!");
    }

    C3DTerm();

    return TRUE;
}

void SessionInit(void)
{
    nResultSetup = SR_OK;
}

void SessionTerm(void)
{
    ShowWindow(hProgressDlg, SW_HIDE);

    if (nMode == MODE_INTERACTIVE)
    {
        switch (nResultSetup)
        {
        case SR_OK:
            FmtMsg(MB_OK | MB_ICONINFORMATION,
                "Setup completed!");
            break;

        case SR_USERCAN:
            FmtMsg(MB_OK | MB_ICONINFORMATION,
                "Setup cancelled!\n\n"
                "Application may not operate correctly.\n\n"
                "Please rerun setup or contact technical support.");
            break;

        case SR_WARNING:
            FmtMsg(MB_OK | MB_ICONEXCLAMATION,
                "Setup completed with warnings!\n\n"
                "Application may not operate correctly.\n\n"
                "Please rerun setup or contact technical support.");
            break;

        case SR_FAILED:
            FmtMsg(MB_OK | MB_ICONHAND,
                "Setup did not complete!\n\n"
                "Application may not operate correctly.\n\n"
                "Please rerun setup or contact technical support.");
            break;
        }
    }

    ShowWindow(hAppWnd, SW_HIDE);
}

HDDEDATA CALLBACK __export DdeCallback(UINT X(type), UINT X(fmt), HCONV X(hconv), HSZ X(hsz1),
                       HSZ X(hsz2), HDDEDATA X(hData), DWORD X(dwData1), DWORD X(dwData2))
{
    return FALSE;
}

#define GL_OK       0
#define GL_EOF      1
#define GL_ERROR    2

#define LT_UNKNOWN  0
#define LT_SECTION  1
#define LT_DATA     2
#define LT_COMMENT  3

#define SS_NONE     -1
#define SS_MAIN     0
#define SS_CONFIG   1
#define SS_FILES    2
#define SS_PROGMAN  3
#define SS_WININI   4
#define SS_REG      5

#define PD_OK       0
#define PD_FAILED   1

int GetLine(HFILE hFileSetup, char * szLineBuf, int nLineBufSize)
{
    int i;
    int nGetLineResult;
    int nReadResult;
    char cFileChar;

    i = 0;
    nGetLineResult = GL_OK;

    for (;;)
    {
        nReadResult = _lread(hFileSetup, &cFileChar, 1);

        if (nReadResult == HFILE_ERROR)
        {
            nGetLineResult = GL_ERROR;
            break;
        }

        if (nReadResult == 0)
        {
            nGetLineResult = GL_EOF;
            break;
        }

        if (cFileChar == '\n')
            break;

        if ((i < (nLineBufSize - 1)) && ((cFileChar == '\t') || (cFileChar >= ' ')))
            szLineBuf[i++] = cFileChar;
    }

    szLineBuf[i] = '\0';

    return nGetLineResult;
}

BOOL XlateLine(char * szLineBuf, char * szXlateLineBuf, int nBufLen)
{
#define XS_COPY 1
#define XS_TOKEN 2

    int nLineBuf;
    int nXlateLineBuf;
    BOOL bSymbol;
    char szSymbol[40];
    int nSymbol;

    nBufLen = 0;    // fix this, it just avoids compiler error now

    bSymbol = FALSE;
    nSymbol = 0;
    nXlateLineBuf = 0;

    for (nLineBuf = 0; szLineBuf[nLineBuf] != '\0'; nLineBuf++)
    {
        if (bSymbol)
        {
            if (szLineBuf[nLineBuf] == '%')
            {
                if (nSymbol == 0)
                {
                    szXlateLineBuf[nXlateLineBuf++] = '%';
                }
                else
                {
                    char * pszVar;

                    szSymbol[nSymbol] = '\0';

                    if ((pszVar = GetVar(szSymbol)) != NULL)
                    {
                        lstrcpy(szXlateLineBuf + nXlateLineBuf, pszVar);
                        nXlateLineBuf += lstrlen(pszVar);
                    }
                }

                bSymbol = FALSE;
            }
            else
            {
                szSymbol[nSymbol++] = szLineBuf[nLineBuf];
            }
        }
        else
        {
            if (szLineBuf[nLineBuf] == '%')
            {
                bSymbol = TRUE;
                nSymbol = 0;
            }
            else
            {
                szXlateLineBuf[nXlateLineBuf++] = szLineBuf[nLineBuf];
            }
        }
    }

    szXlateLineBuf[nXlateLineBuf] = '\0';

    return TRUE;
}

int LineType(char * szLineBuf)
{
    int i;
    int nLineType;

    nLineType = LT_UNKNOWN;

    for (i = 0; szLineBuf[i] == ' ' || szLineBuf[i] == '\t'; i++);

    switch (szLineBuf[i])
    {
    case '\0':
    case ';':
        nLineType = LT_COMMENT;
        break;

    case '[':
        nLineType = LT_SECTION;
        break;

    default:
        nLineType = LT_DATA;
        break;
    }

    return nLineType;
}

int Parse(char * pszLineBuf, char * pszFormat, ...)
{
    int nResultParse;
    int nFormatPos;
    int nLineBufPos;
    int nTempLineBufPos;
    va_list pArgList;

    nResultParse = 0;
    nLineBufPos = 0;
    va_start(pArgList, pszFormat);

    for (nFormatPos = 0; pszFormat[nFormatPos] != '\0'; nFormatPos++)
    {
        if (pszFormat[nFormatPos] == 'T')
        {
            char * * ppszToken;

            /* Record the start of the next token at current position. */
            ppszToken = va_arg(pArgList, char * *);
            *ppszToken = (pszLineBuf + nLineBufPos);
            if (pszLineBuf[nLineBufPos] != '\0')
                nResultParse++;
        }
        else
        {
            int nLineBufStartPos;

            nLineBufStartPos = nLineBufPos;

            if (pszLineBuf[nLineBufPos] == '\0')
                continue;

            /* Find delimiter */
            /* Eventually this needs to understand string quoting */
            for (; pszLineBuf[nLineBufPos] != pszFormat[nFormatPos] &&
                   pszLineBuf[nLineBufPos] != '\0'; nLineBufPos++);

            /* Backup gobbling white space, if any */
            for (nTempLineBufPos = nLineBufPos;
                 ((nTempLineBufPos > nLineBufStartPos) &&
                  ((pszLineBuf[nTempLineBufPos - 1] == ' ') ||
                   (pszLineBuf[nTempLineBufPos - 1] == '\t')));
                 nTempLineBufPos--);

            /* Now positioned on character following token */

            /* If original delimiter is null, use special case.  Insert
               terminator at temp pos in case there was trailing
               white space, then break out. */
            if (pszLineBuf[nLineBufPos] == '\0')
            {
                pszLineBuf[nTempLineBufPos] = '\0';
                continue;
            }

            /* replace with null term */
            pszLineBuf[nTempLineBufPos] = '\0';

            /* Bump over delimiter found */
            nLineBufPos++;

            /* Skip white space, if any */
            for (; pszLineBuf[nLineBufPos] == ' ' || pszLineBuf[nLineBufPos] == '\t';
                 nLineBufPos++);
        }
    }

    return nResultParse;
}

int IdentifySection(char * szLineBuf, int nLine)
{
    int nSection;
    int nParse;
    char * pszSectionName;

    nParse = Parse(szLineBuf, "[T]", &pszSectionName);

    if (nParse < 1)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return SS_NONE;
    }

    /* Match section name.  Should be table driven! */

    nSection = SS_NONE;
    if (lstrcmpi(pszSectionName, "Main") == 0)
        nSection = SS_MAIN;
    if (lstrcmpi(pszSectionName, "Config") == 0)
        nSection = SS_CONFIG;
    if (lstrcmpi(pszSectionName, "Files") == 0)
        nSection = SS_FILES;
    if (lstrcmpi(pszSectionName, "WinIni") == 0)
        nSection = SS_WININI;
    if (lstrcmpi(pszSectionName, "ProgMan") == 0)
        nSection = SS_PROGMAN;
    if (lstrcmpi(pszSectionName, "Reg") == 0)
        nSection = SS_REG;

    return nSection;
}

int MainStart(void)
{
    if (bMainProcessed)
        SessionTerm();

    SessionInit();

    szAppTitle[0] = '\0';
    szDestDir[0] = '\0';

    SetWindowText(hAppWnd, szAppName);

    return 0;
}

int MainSetVar(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszKey;
    char * pszValue;

    nParse = Parse(szLineBuf, "T:T=T;", &pszCmd, &pszKey, &pszValue);

    if (nParse < 3)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    SetVar(pszKey, pszValue);

    return PD_OK;
}

int MainMode(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszValue;

    nParse = Parse(szLineBuf, "T:T;", &pszCmd, &pszValue);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    if (lstrcmpi(pszValue, "Interactive") == 0)
        nMode = MODE_INTERACTIVE;
    else if (lstrcmpi(pszValue, "Batch") == 0)
        nMode = MODE_BATCH;
    else if (lstrcmpi(pszValue, "Stealth") == 0)
        nMode = MODE_STEALTH;
    else
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nInvalid mode value!", nLine);
        return PD_FAILED;
    }

    return PD_OK;
}

int MainGetIniVar(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszValue;
    char * pszSection;
    char * pszKey;

    nParse = Parse(szLineBuf, "T:T=T,T;", &pszCmd, &pszValue, &pszSection, &pszKey);

    if (nParse < 4)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    GetProfileString(pszSection, pszKey, "", GetVar(pszValue), sizeof(PATHNAM));

    return PD_OK;
}

#ifdef _WIN32

int MainGetRegVar(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszValue;
    char * pszKey;
    char * pszSubKey;
    char * pszValName;
    HKEY hKey;
    HKEY hSubKey;
    LONG lResult;
    DWORD dwType;
    DWORD dwDataSize;

    nParse = Parse(szLineBuf, "T:T=T,T,T;", &pszCmd, &pszValue, &pszKey, &pszSubKey, &pszValName);

    if (nParse < 4)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    if (lstrcmpi(pszKey, "HKEY_CLASSES_ROOT") == 0)
        hKey = HKEY_CLASSES_ROOT;
    else if (lstrcmpi(pszKey, "HKEY_LOCAL_MACHINE") == 0)
        hKey = HKEY_LOCAL_MACHINE;
    else
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nUnrecognized HKEY!", nLine);
        hKey = NULL;
    }

    lResult = RegOpenKey(hKey, pszSubKey, &hSubKey);

    dwDataSize = sizeof(szDestDir);

    lResult = RegQueryValueEx(hSubKey, pszValName, 0, &dwType,
        (BYTE *)GetVar(pszValue), &dwDataSize);

    lResult = RegCloseKey(hSubKey);

    return PD_OK;
}

#endif

int MainRunAtExit(char *szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszExecCmdLine;

    nParse = Parse(szLineBuf, "T:T;", &pszCmd, &pszExecCmdLine);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    lstrcpy(szRunAtExitCmdLine, pszExecCmdLine);

    return PD_OK;
}

int MainEnd(void)
{
    bMainProcessed = TRUE;

    SetWindowText(hAppWnd, szAppTitle);

    ShowWindow(hAppWnd, nMode == MODE_STEALTH ? SW_HIDE : SW_MAXIMIZE);
    UpdateWindow(hAppWnd);

    return 0;
}

int ConfigStart(void)
{
    ShowWindow(hProgressDlg, SW_HIDE);

    return 0;
}

int ConfigGetDir(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszVarName;
    char * pszPrompt;

    nParse = Parse(szLineBuf, "T:T,T;", &pszCmd, &pszVarName, &pszPrompt);

    if (nParse < 3)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    if (szDestDir[0] == '\0')
        lstrcpy(szDestDir, "C:\\");
    lstrcpy(szDirDlgDirectory, szDestDir);
    lstrcpy(szDirDlgPrompt, pszPrompt);

    if (GoDialogBox("DirDlgBox", hProgressDlg, (FARPROC)DirDlgProc))
        lstrcpy(szDestDir, szDirDlgDirectory);
    else
        return PD_FAILED;

    return PD_OK;
}

int ConfigConfirm(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszPrompt;
    int nResult;

    nParse = Parse(szLineBuf, "T:T;", &pszCmd, &pszPrompt);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    nResult = FmtMsg(MB_OKCANCEL | MB_ICONQUESTION, pszPrompt);
    if (nResult == IDCANCEL)
    {
        bAbort = TRUE;
        nResultSetup = SR_USERCAN;
    }

    return PD_OK;
}

int ConfigEnd(void)
{
    return 0;
}

int FilesStart(void)
{
    ShowWindow(hProgressDlg, nMode == MODE_STEALTH ? SW_HIDE : SW_SHOW);

    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "Copying Files...");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");

    return 0;
}

int FilesCopyFile(char *szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszSrcFilename;
    char * pszDestFilename;
    char * pszSysFlag;
    BOOL bSysFile;
    char szTmpFile [80];
    UINT wTmpFileLen;
    DWORD dwResult;
    int  nResult;

    nParse = Parse(szLineBuf, "T:T,T,T;", &pszCmd, &pszSrcFilename, &pszDestFilename, &pszSysFlag);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    bSysFile = (lstrcmpi(pszSysFlag, "System") == 0);

    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, pszSrcFilename);

    szTmpFile[0] = '\0';
    wTmpFileLen = sizeof(szTmpFile);

    if ((pszDestFilename == NULL) || (pszDestFilename[0] == '\0'))
        pszDestFilename = pszSrcFilename;

    while ((dwResult = VerInstallFile(bSysFile ? 0 : VIFF_FORCEINSTALL,
                              pszSrcFilename,
                              pszDestFilename,
                              szSrcDir,
                              bSysFile ? szSysDir : szDestDir,
                              NULL,
                              szTmpFile,
                              &wTmpFileLen)) != 0) {

        wsprintf(szBuf, "Error copying %s\\%s to %s\\%s\n",
                 (LPSTR)szSrcDir, (LPSTR)pszSrcFilename,
                 (LPSTR)(bSysFile ? szSysDir : szDestDir),
                 (LPSTR)pszDestFilename);

        if (dwResult & VIF_TEMPFILE)
            lstrcat(szBuf, "\nTemporary File Created");
        if (dwResult & VIF_MISMATCH)
            lstrcat(szBuf, "\nNew file attribute(s) differ from current file");
        if (dwResult & VIF_SRCOLD)
            lstrcat(szBuf, "\nNew file is older version than current file");
        if (dwResult & VIF_DIFFLANG)
            lstrcat(szBuf, "\nNew file has different language or code page from current file");
        if (dwResult & VIF_DIFFCODEPG)
            lstrcat(szBuf, "\nNew file requires unavailable code page");
        if (dwResult & VIF_DIFFTYPE)
            lstrcat(szBuf, "\nNew file has different type, subtype, or operating system from current file");
        if (dwResult & VIF_WRITEPROT)
            lstrcat(szBuf, "\nCurrent file is write protected");
        if (dwResult & VIF_FILEINUSE)
            lstrcat(szBuf, "\nCurrent file is in use");
        if (dwResult & VIF_OUTOFSPACE)
            lstrcat(szBuf, "\nOut of space on destination drive");
        if (dwResult & VIF_ACCESSVIOLATION)
            lstrcat(szBuf, "\nAccess violation");
        if (dwResult & VIF_SHARINGVIOLATION)
            lstrcat(szBuf, "\nSharing violation");
        if (dwResult & VIF_CANNOTCREATE)
            lstrcat(szBuf, "\nCan not create destination file");
        if (dwResult & VIF_CANNOTDELETE)
            lstrcat(szBuf, "\nCan not delete previous instance of destination file");
        if (dwResult & VIF_CANNOTDELETECUR)
            lstrcat(szBuf, "\nCan not delete previous instance of destination file");
        if (dwResult & VIF_CANNOTRENAME)
            lstrcat(szBuf, "\nCan not rename temporary file");
        if (dwResult & VIF_OUTOFMEMORY)
            lstrcat(szBuf, "\nOut of memory");
        if (dwResult & VIF_CANNOTREADSRC)
            lstrcat(szBuf, "\nCan not read  source file");
        if (dwResult & VIF_CANNOTREADDST)
            lstrcat(szBuf, "\nCan not read previous instance of destination file");
        if (dwResult & VIF_BUFFTOOSMALL)
            lstrcat(szBuf, "\nTemporary file name buffer too small (internal error)");

        nResult = FmtMsg(MB_ABORTRETRYIGNORE | MB_ICONQUESTION, szBuf);

        if (nResult == IDABORT) {
            bAbort = TRUE;
            nResultSetup = SR_USERCAN;
            break;
        }

        if (nResult == IDIGNORE)
            break;
    }

    return PD_OK;
}

int FilesDelFile(char *szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszFileName;
    char * pszDelayFlag;

    nParse = Parse(szLineBuf, "T:T,T;", &pszCmd, &pszFileName, &pszDelayFlag);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "%s: %s",
                      (LPSTR)pszCmd, (LPSTR)pszFileName);

    if ((pszDelayFlag != NULL) && (lstrcmpi(pszDelayFlag, "Delay") == 0))
    {
        if (!ReplaceFileOnReboot(pszFileName, NULL))
        {
            FmtMsg(MB_OK | MB_ICONHAND,
                   "Can't delay delete file %s!", (LPSTR)pszFileName);
            return PD_FAILED;
        }
    }
    else
    {
        if (remove(pszFileName) == -1)
        {
            FmtMsg(MB_OK | MB_ICONHAND,
                   "Can't delete file %s!", (LPSTR)pszFileName);
            return PD_FAILED;
        }
    }

    return PD_OK;
}

int FilesDelDir(char *szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszDirName;
    char * pszDelayFlag;

    nParse = Parse(szLineBuf, "T:T,T;", &pszCmd, &pszDirName, &pszDelayFlag);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "%s: %s",
                      (LPSTR)pszCmd, (LPSTR)pszDirName);

    if ((pszDelayFlag != NULL) && (lstrcmpi(pszDelayFlag, "Delay") == 0))
    {
        if (!ReplaceFileOnReboot(pszDirName, NULL))
        {
            FmtMsg(MB_OK | MB_ICONHAND,
                   "Can't delay remove directory %s!", (LPSTR)pszDirName);
            return PD_FAILED;
        }
    }
    else
    {
        if (_rmdir(pszDirName) == -1)
        {
            FmtMsg(MB_OK | MB_ICONHAND,
                   "Can't remove directory %s!", (LPSTR)pszDirName);
            return PD_FAILED;
        }
    }

    return PD_OK;
}

int FilesWinExec(char *szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszExecCmdLine;

    nParse = Parse(szLineBuf, "T:T;", &pszCmd, &pszExecCmdLine);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    WinExec(pszExecCmdLine, SW_SHOWNORMAL);

    return PD_OK;
}

int FilesEnd(void)
{
    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");

    return 0;
}

int WinIniStart(void)
{
    ShowWindow(hProgressDlg, nMode == MODE_STEALTH ? SW_HIDE : SW_SHOW);

    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "Updating WIN.INI...");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");

    return 0;
}

int WinIniAddVal(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszSection;
    char * pszKey;
    char * pszValue;

    nParse = Parse(szLineBuf, "T:T,T=T;", &pszCmd, &pszSection, &pszKey, &pszValue);

    if (nParse < 4)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    WriteProfileString(pszSection, pszKey, pszValue);

    return PD_OK;
}

int WinIniDelKey(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszSection;
    char * pszKey;

    nParse = Parse(szLineBuf, "T:T,T;", &pszCmd, &pszSection, &pszKey);

    if (nParse < 3)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    WriteProfileString(pszSection, pszKey, NULL);

    return PD_OK;
}

int WinIniDelSec(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszSection;

    nParse = Parse(szLineBuf, "T:T;", &pszCmd, &pszSection);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    WriteProfileString(pszSection, NULL, NULL);

    return PD_OK;
}

int WinIniEnd(void)
{
    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");

    return 0;
}

int ProgManStart(void)
{
    int   nResultFunc;
    WORD wResult;
    HSZ   hszApplication;
    HSZ   hszTopic;

    ShowWindow(hProgressDlg, nMode == MODE_STEALTH ? SW_HIDE : SW_SHOW);

    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "Adding/Updating Program Manager Icons...");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "Connecting");

    nResultFunc = TRUE;

    lpDdeProc = MakeProcInstance((FARPROC)DdeCallback, hAppInst);
    if (lpDdeProc == NULL) {
        FmtMsg(MB_OK | MB_ICONSTOP, "Can't create  DDE Callback Proc Instance!");
        return FALSE;
    }

    wResult = (WORD)DdeInitialize(&idInst, (PFNCALLBACK)lpDdeProc, APPCMD_CLIENTONLY, 0L);
    if (wResult != DMLERR_NO_ERROR)
    {
        FmtMsg(MB_OK | MB_ICONSTOP, "Can't initialize DDE!");
        return FALSE;
    }

    hszApplication = DdeCreateStringHandle(idInst, "PROGMAN", 0);
    hszTopic = DdeCreateStringHandle(idInst, "PROGMAN", 0);

    if ((hszApplication != NULL) && (hszTopic != NULL))
        hConv = DdeConnect(idInst, hszApplication, hszTopic, NULL);
    else
        hConv = NULL;

    DdeFreeStringHandle(idInst, hszTopic);
    DdeFreeStringHandle(idInst, hszApplication);

    if (hConv == NULL)
    {
        FmtMsg(MB_OK | MB_ICONSTOP, "Can't establish DDE connection to Program Manager!");
        return FALSE;
    }

    return nResultFunc;
}

int ProgManCreateGroup(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszGroupName;
    char * pszGroupFile;
    DWORD dwResult;
    DWORD dwStatus;

    nParse = Parse(szLineBuf, "T:T,T;", &pszCmd, &pszGroupName, &pszGroupFile);

    if (nParse < 3)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "%s: %s",
                      (LPSTR)pszCmd, (LPSTR)pszGroupName);

    wsprintf(szBuf, "[CreateGroup(\"%s\", \"%s\")]",
             (LPSTR)pszGroupName, (LPSTR)pszGroupFile);

    dwResult = (DWORD)DdeClientTransaction((unsigned char *)szBuf,
                                        lstrlen(szBuf) + 1, hConv, NULL,
                                       CF_TEXT, XTYP_EXECUTE, 3000, &dwStatus);

    return PD_OK;
}

int ProgManDeleteGroup(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszGroupName;
    DWORD dwResult;
    DWORD dwStatus;

    nParse = Parse(szLineBuf, "T:T;", &pszCmd, &pszGroupName);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "%s: %s",
                      (LPSTR)pszCmd, (LPSTR)pszGroupName);

    wsprintf(szBuf, "[DeleteGroup(\"%s\")]",
             (LPSTR)pszGroupName);

    dwResult = (DWORD)DdeClientTransaction((unsigned char *)szBuf,
                                        lstrlen(szBuf) + 1, hConv, NULL,
                                       CF_TEXT, XTYP_EXECUTE, 3000, &dwStatus);

    return PD_OK;
}

int ProgManAddItem(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszArg1;
    char * pszArg2;
    DWORD dwResult;
    DWORD dwStatus;

    nParse = Parse(szLineBuf, "T:T,T;", &pszCmd, &pszArg1, &pszArg2);

    if (nParse < 3)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "%s: %s",
                      (LPSTR)pszCmd, (LPSTR)pszArg1);

    wsprintf(szBuf, "[ReplaceItem(%s)]",
             (LPSTR)pszArg1);
    dwResult = (DWORD)DdeClientTransaction((unsigned char *)szBuf, lstrlen(szBuf) + 1, hConv, NULL,
                                       CF_TEXT, XTYP_EXECUTE, 3000, &dwStatus);

    wsprintf(szBuf, "[AddItem(%s, %s)]",
             (LPSTR)pszArg1,
             (LPSTR)pszArg2);
    dwResult = (DWORD)DdeClientTransaction((unsigned char *)szBuf, lstrlen(szBuf) + 1, hConv, NULL,
                                           CF_TEXT, XTYP_EXECUTE, 3000, &dwStatus);

    return PD_OK;
}

int ProgManDeleteItem(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszItemName;
    DWORD dwResult;
    DWORD dwStatus;

    nParse = Parse(szLineBuf, "T:T;", &pszCmd, &pszItemName);

    if (nParse < 2)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "%s: %s",
                      (LPSTR)pszCmd, (LPSTR)pszItemName);

    wsprintf(szBuf, "[DeleteItem(\"%s\")]",
             (LPSTR)pszItemName);

    dwResult = (DWORD)DdeClientTransaction((unsigned char *)szBuf,
                                        lstrlen(szBuf) + 1, hConv, NULL,
                                       CF_TEXT, XTYP_EXECUTE, 3000, &dwStatus);

    return PD_OK;
}

int ProgManEnd(void)
{
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "Disconnecting");

    if (hConv != NULL)
    {
        DdeDisconnect(hConv);
        hConv = NULL;
    }

    if (idInst != 0)
    {
        DdeUninitialize(idInst);
        idInst = 0;
    }

    if (lpDdeProc != NULL)
    {
        X32(FreeProcInstance(lpDdeProc));
        lpDdeProc = NULL;
    }

    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");

    return TRUE;
}

int RegStart(void)
{
    ShowWindow(hProgressDlg, nMode == MODE_STEALTH ? SW_HIDE : SW_SHOW);

    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "Adding/Updating Registry Entries...");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");

    return 0;
}

int RegAddVal(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszKey;
    char * pszSubKey;
    char * pszValue;
    HKEY hKey;

    nParse = Parse(szLineBuf, "T:T,T=T;", &pszCmd, &pszKey, &pszSubKey, &pszValue);

    if (nParse < 3)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    if (lstrcmpi(pszKey, "HKEY_CLASSES_ROOT") == 0)
        hKey = HKEY_CLASSES_ROOT;
#ifdef _WIN32
    else if (lstrcmpi(pszKey, "HKEY_LOCAL_MACHINE") == 0)
        hKey = HKEY_LOCAL_MACHINE;
#endif
    else
    {
        hKey = NULL;
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nUnrecognized HKEY!", nLine);
    }

    RegSetValue(hKey, pszSubKey, REG_SZ,
                pszValue, lstrlen(pszValue));

    return PD_OK;
}

#ifdef _WIN32
int RegSetValEx(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszKey;
    char * pszSubKey;
    char * pszValName;
    char * pszValue;
    HKEY hKey;
    HKEY hSubKey;
    LONG lResult;

    nParse = Parse(szLineBuf, "T:T,T,T=T;", &pszCmd, &pszKey, &pszSubKey, &pszValName, &pszValue);

    if (nParse < 4)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    if (lstrcmpi(pszKey, "HKEY_CLASSES_ROOT") == 0)
        hKey = HKEY_CLASSES_ROOT;
    else if (lstrcmpi(pszKey, "HKEY_LOCAL_MACHINE") == 0)
        hKey = HKEY_LOCAL_MACHINE;
    else
        hKey = NULL;

    lResult = RegCreateKey(hKey, pszSubKey, &hSubKey);

    lResult = RegSetValueEx(hSubKey, pszValName, 0, REG_SZ,
                (BYTE *)pszValue, lstrlen(pszValue));

    lResult = RegCloseKey(hSubKey);

    return PD_OK;
}

#endif

/* The following ugly recursive function is used because
   the WIN32 doc indicates that NT will fail a request
   to delete a registry key if it has any children.  So,
   we just enumerate and delete recursively. */

LONG RegDeleteKeyAll(HKEY hKey, LPCSTR pszSubKey)
{
    int i;
    DWORD nKeyCount;
    HKEY hSubKey;
    LONG lResult;
    char szEnumKey[64];


    lResult = RegOpenKey(hKey, pszSubKey, &hSubKey);
    if (lResult != ERROR_SUCCESS)
        return lResult;

    // Kludgy to find number of subkeys.  Only way WIN16 supports. */
    for (i = 0; RegEnumKey(hSubKey, i, szEnumKey, sizeof(szEnumKey)) == ERROR_SUCCESS; i++);
    nKeyCount = i;

    // Step backwards so index doesn't get screwed up as keys are deleted. */
    for (i = (int)nKeyCount - 1; i >= 0; i--)
    {
        char szEnumKey2[64];

        if (RegEnumKey(hSubKey, i, szEnumKey2, sizeof(szEnumKey2)) == ERROR_SUCCESS)
        {
            char szFullChildKey[256];

            lstrcpy(szFullChildKey, pszSubKey);
            lstrcat(szFullChildKey, "\\");
            lstrcat(szFullChildKey, szEnumKey2);

            lResult = RegDeleteKeyAll(hKey, szFullChildKey);
        }
    }

    lResult = RegCloseKey(hSubKey);

    lResult = RegDeleteKey(hKey, pszSubKey);

    return lResult;
}

int RegDelKey(char * szLineBuf, int nLine)
{
    int nParse;
    char * pszCmd;
    char * pszKey;
    char * pszSubKey;
    HKEY hKey;

    nParse = Parse(szLineBuf, "T:T,T;", &pszCmd, &pszKey, &pszSubKey);

    if (nParse < 3)
    {
        FmtMsg(MB_OK | MB_ICONHAND,
               "Syntax error @ line %d\n\nMissing token(s)!", nLine);
        return PD_FAILED;
    }

    if (lstrcmpi(pszKey, "HKEY_CLASSES_ROOT") == 0)
        hKey = HKEY_CLASSES_ROOT;
#ifdef _WIN32
    else if (lstrcmpi(pszKey, "HKEY_LOCAL_MACHINE") == 0)
        hKey = HKEY_LOCAL_MACHINE;
#endif
    else
        hKey = NULL;

    RegDeleteKeyAll(hKey, pszSubKey);
    return PD_OK;
}

int RegEnd(void)
{
    SetDlgItemFmtText(hProgressDlg, IDC_ACTION, "");
    SetDlgItemFmtText(hProgressDlg, IDC_SCALE, "");

    return 0;
}

typedef struct
{
    char * pszCommand;
    int (*Process)(char *, int);
} CMDMAP;

CMDMAP MainCmdMap[] =
{
    {"Mode",        MainMode},
    {"SetVar",      MainSetVar},
    {"GetIniVar",   MainGetIniVar},
#ifdef _WIN32
    {"GetRegVar",   MainGetRegVar},
#endif
    {"RunAtExit",   MainRunAtExit},
    {NULL, NULL}
};

CMDMAP ConfigCmdMap[] =
{
    {"GetDir",      ConfigGetDir},
    {"Confirm",     ConfigConfirm},
    {NULL, NULL}
};

CMDMAP FilesCmdMap[] =
{
    {"CopyFile",    FilesCopyFile},
    {"DelFile",     FilesDelFile},
    {"DelDir",      FilesDelDir},
    {"WinExec",     FilesWinExec},
    {NULL, NULL}
};

CMDMAP WinIniCmdMap[] =
{
    {"AddVal",      WinIniAddVal},
    {"DelKey",      WinIniDelKey},
    {"DelSec",      WinIniDelSec},
    {NULL, NULL}
};

CMDMAP ProgManCmdMap[] =
{
    {"CreateGroup", ProgManCreateGroup},
    {"DeleteGroup", ProgManDeleteGroup},
    {"AddItem",     ProgManAddItem},
    {"DeleteItem",  ProgManDeleteItem},
    {NULL, NULL}
};

CMDMAP RegCmdMap[] =
{
    {"AddVal",      RegAddVal},
#ifdef _WIN32
    {"SetValEx",    RegSetValEx},
#endif
    {"DelKey",      RegDelKey},
    {NULL,          NULL}
};

typedef struct
{
    int (*Start)(void);
    CMDMAP * pCmdMap;
    int (*End)(void);
} SECPROCS;

SECPROCS SecProcs[] =
{
    {MainStart,     MainCmdMap,     MainEnd},
    {ConfigStart,   ConfigCmdMap,   ConfigEnd},
    {FilesStart,    FilesCmdMap,    FilesEnd},
    {ProgManStart,  ProgManCmdMap,  ProgManEnd},
    {WinIniStart,   WinIniCmdMap,   WinIniEnd},
    {RegStart,      RegCmdMap,      RegEnd}
};

int InvokeDataCmd(CMDMAP * pCmdMap, char * szLineBuf, int nLine)
{
    char szCommand[40];
    int i;
    int nCmdEntry;

    for (i = 0; (i < (sizeof(szCommand) - 1)) &&
                (szLineBuf[i] != '\0') &&
                (szLineBuf[i] != ':'); i++)
        szCommand[i] = szLineBuf[i];

    szCommand[i] = '\0';

    for (nCmdEntry = 0;
         (pCmdMap[nCmdEntry].pszCommand != NULL) &&
         (lstrcmpi(szCommand, pCmdMap[nCmdEntry].pszCommand) != 0);
         nCmdEntry++);

    if (pCmdMap[nCmdEntry].pszCommand == NULL)
    {
        FmtMsg(MB_OK | MB_ICONSTOP,
              "Command not found!");
        return PD_FAILED;
    }
    else
        return pCmdMap[nCmdEntry].Process(szLineBuf, nLine);
}

void SetupApplication(HFILE hFileSetup)
{
    int     nResultGetLine;
    int     nSection;
    int     nLine;
    char    szLineBuf[1024];

    nLine = 0;

    if (!bAbort)
    {
        SessionInit();

        nSection = SS_NONE;

        do
        {
            nResultGetLine = GetLine(hFileSetup, szLineBuf, sizeof(szLineBuf));

            /* If result is OK or EOF, process the line (there may still be
            line data to process at EOF).  Anything else is an error,
            so break out of loop and assume error will be diagnosed later. */

            if (!((nResultGetLine == GL_OK) || (nResultGetLine == GL_EOF)))
                break;

            nLine++;

            switch (LineType(szLineBuf))
            {
            case LT_SECTION:
                {
                    if (nSection != SS_NONE)
                        (*(SecProcs[nSection].End))();

                    nSection = IdentifySection(szLineBuf, nLine);

                    if (nSection != SS_NONE && nSection != SS_MAIN)
                    {
                        if (!bMainProcessed)
                        {
                            FmtMsg(MB_OK | MB_ICONSTOP,
                                "Setup control file must start with Main section!");
                            bAbort = TRUE;
                        }
                    }

                    if (nSection != SS_NONE)
                        (*(SecProcs[nSection].Start))();

                    break;
                }

            case LT_DATA:
                {
                    char szXlateLineBuf[sizeof(szLineBuf)];

                    XlateLine(szLineBuf, szXlateLineBuf, sizeof(szXlateLineBuf));

                    if (nSection != SS_NONE)
                        InvokeDataCmd(SecProcs[nSection].pCmdMap, szXlateLineBuf, nLine);
//                      SecProcs[nSection].Process(szXlateLineBuf, nLine);

                    break;
                }

            case LT_COMMENT:
                break;

            case LT_UNKNOWN:
                break;

            default:
                break;
            }

            HandleMessages();
        }
        while (!bAbort && nResultGetLine == GL_OK);

        if (nSection != SS_NONE)
            (*(SecProcs[nSection].End))();

        if (nResultGetLine == GL_ERROR)
        {
            nResultSetup = SR_FAILED;
            FmtMsg(MB_OK | MB_ICONSTOP, "Error reading setup control file file!");
        }

        SessionTerm();
    }
}

void ProcessSetupCmdFile(LPSTR szSetupFile)
{
    HFILE hFileSetup;

    if (szSetupFile == NULL || szSetupFile[0] == '\0')
        wsprintf(szBuf, "%s\\Install.scf", (LPSTR)szSrcDir);
    else
        wsprintf(szBuf, "%s\\%s", (LPSTR)szSrcDir, (LPSTR)szSetupFile);

    hFileSetup = _lopen(szBuf, OF_READ);

    if (hFileSetup != HFILE_ERROR)
    {
        SetupApplication(hFileSetup);

        _lclose(hFileSetup);
    }
    else /* hFileSetup == HFILE_ERROR */
    {
        FmtMsg(MB_OK | MB_ICONSTOP,
            "Error opening setup command file %s - "
            "%s\n\n"
            "Setup can't continue.", (LPSTR)szBuf,
            (LPSTR)StrOSErr(W16W32(errno, GetLastError())));
    }
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
    if (AppInit(hInstance, hPrevInstance, lpszCmdLine, nCmdShow))
        ProcessSetupCmdFile(szSetupCmdFile);

    AppTerm();

    return 0;
}
