/*******************************************************************************
**                                                                            **
**                      Kermit for Microsoft Windows                          **
**                      ----------------------------                          **
**                               KERMTTY.C                                    **
**                                                                            **
**                      Version 0.50 - Developmental                          **
**                     Requires Microsoft Windows 2.X                         **
**                         Author: Wayne Warthen                              **
**                                                                            **
**  Kermit Terminal Emulation Module -- Teletype                              **
**                                                                            **
*******************************************************************************/

/* DEFINITIONS ---------------------------------------------------------------*/

#define GLOBAL

#define ANSI_BLACK   0
#define ANSI_RED     1
#define ANSI_GREEN   2
#define ANSI_YELLOW  3
#define ANSI_BLUE    4
#define ANSI_MAGENTA 5
#define ANSI_CYAN    6
#define ANSI_WHITE   7

#define DEFATTR     ((BYTE)(ATTR_FGWHITE | ATTR_BGBLACK))

/* INCLUDES ------------------------------------------------------------------*/

#include "kermtty.h"
#include <string.h>
#include <stdlib.h>

/* TYPEDEFS ------------------------------------------------------------------*/

typedef struct {
    char * szKeyName;
    UINT   nKeyID;
} TTYKEYMAP;

/* LOCAL STORAGE -------------------------------------------------------------*/

static TTYKEYMAP tkm[] = {
    {NULL, 0}
};

static KEYMAP km[] = {{0, 0, 0, 0, NULL}};

#ifdef _WIN32

/*----------------------------------------------------------------------------*/
extern "C" int APIENTRY DllMain
/*-----------------------------------------------------------------------------
**
**  Library initialization procedure called when library is loaded.
*/
(HINSTANCE     hInst,         /* Instance handle for library.                 */
 DWORD,        X(ul_reason)   /* Handle to the local data segment.            */
 LPVOID)       X(lpReserved)  /* Size of local heap.                          */
/*----------------------------------------------------------------------------*/

{
    hLibrary = hInst;
    return 1;
}

#else

/*----------------------------------------------------------------------------*/
int CALLBACK LibMain
/*-----------------------------------------------------------------------------
**
**  Library initialization procedure called when library is loaded.
*/
(HINSTANCE    hInstance,      /* Instance handle for library.                 */
 WORD         X(wDataSeg),    /* Handle to the local data segment.            */
 WORD         cbHeapSize,     /* Size of local heap.                          */
 LPSTR        X(lpszCmdLine)) /* Pointer to command line.                     */
/*----------------------------------------------------------------------------*/

{
    hLibrary = hInstance;

    if (cbHeapSize != 0)
        UnlockData(0);

    return(TRUE);
}
#endif

/*----------------------------------------------------------------------------*/
VOID WriteTermChar
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Handle a character being sent to the terminal.  This function handles     */
/*  control sequences as well as characters to be written.                    */
/*                                                                            */
(char TermChar,     /* Character for terminal.                                */
 BOOL Raw)          /* Raw mode flag (suppress formatting when TRUE).         */
/*----------------------------------------------------------------------------*/

{
    if (Raw || TermChar >= 32) {
        if (npTCB->nCurCol >= TERMCOLS) {
            if (npTCB->TermSet.LineWrap) {
                WriteTermChar('\r', FALSE);
                WriteTermChar('\n', FALSE);
            }
            else
                npTCB->nCurCol = LASTCOL;
        }

        WriteScrnChar((npTCB->nCurCol)++, npTCB->nCurRow, TermChar);
        if (!npTCB->TermSet.LineWrap && (npTCB->nCurCol >= TERMCOLS))
            npTCB->nCurCol = LASTCOL;

        return;
    }

    switch(TermChar) {
        case 0:                     /* nul */
            break;

        case 7:                     /* bell */
            MessageBeep(0);
            break;

        case 8:                     /* backspace */
            if (npTCB->nCurCol > 0)
                (npTCB->nCurCol)--;
            break;

        case 9:                     /* tab */
            npTCB->nCurCol = min(LASTCOL,
                                 npTCB->nCurCol + 8 - (npTCB->nCurCol % 8));
            break;

        case 10:                    /* line feed */
            if (npTCB->nCurRow + 1 < TERMROWS)
                (npTCB->nCurRow)++;
            else
                ScrollScrn(0, 0, NULL);
            break;

        case 12:                    /* form feed */
            ScrollScrn(0, TERMROWS, NULL);
            FillScrn(NULL, ' ');
            FillAttr(NULL, DEFATTR);
            npTCB->nCurRow = 0;
            npTCB->nCurCol = 0;
            break;

        case 13:                    /* carriage return */
            npTCB->nCurCol = 0;
            break;

        default:
            WriteTermChar(TermChar, TRUE);
            break;
    }
}

/*----------------------------------------------------------------------------*/
extern "C"  VOID DLLEXPORT CALLBACK WriteTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Process string of characters for the terminal.                            */
/*                                                                            */
(HANDLE hEmul,      /* Handle to callers terminal window.                     */
 LPSTR  TermStr,    /* Character string to be written.                        */
 int    StrLen,     /* Number of chars to write (string may contain nulls).   */
 BOOL   Raw)        /* Raw mode flag (suppress formatting when TRUE).         */
/*----------------------------------------------------------------------------*/

{
    int i;

    npTCB = (TCB *)LocalLock(hEmul);

    for (i = 0; i < StrLen; i++)
        WriteTermChar(TermStr[i], Raw);

    FlushScrnBuf();
    SetCurPos(npTCB->nCurCol, npTCB->nCurRow, FALSE);

    LocalUnlock(hEmul);
}

/*----------------------------------------------------------------------------*/
extern "C" VOID DLLEXPORT CALLBACK ProcessTermChar
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Process a character entered at the terminal console.  Function sends      */
/*  the character down the comm line, then echoes the character to the        */
/*  terminal is echoing is enabled.                                           */
/*                                                                            */
(HANDLE hEmul,  /* Handle to callers terminal window.                         */
 char cChar)    /* Character entered.                                         */
/*----------------------------------------------------------------------------*/

{
    npTCB = (TCB *)LocalLock(hEmul);

    SendTermChar(cChar);
    if (npTCB->TermSet.LocalEcho)
        WriteTerm(hEmul, &cChar, 1, FALSE);

    if (cChar == '\r' && npTCB->TermSet.NewLine) {
        cChar = '\n';
        SendTermChar(cChar);
        if (npTCB->TermSet.LocalEcho)
            WriteTerm(hEmul, &cChar, 1, FALSE);
    }

    LocalUnlock(hEmul);
}

/*----------------------------------------------------------------------------*/
extern "C" VOID DLLEXPORT CALLBACK ProcessTermKey
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Process a key entered at the terminal console.  Function handles          */
/*  function keys, etc.  Echoes if appropriate.                               */
/*                                                                            */
(HANDLE, /* hEmul */ /* Handle to callers terminal window.                    */
 UINT)   /* cKey */  /* Key pressed.                                          */
/*----------------------------------------------------------------------------*/

{
}

extern "C" BOOL DLLEXPORT CALLBACK DoKeyMap(HANDLE X(hEmul), KEYMAPENTRY FAR * lpkme)
{
    int    i;
    KEYMAP km2;

    for (i = 0; tkm[i].szKeyName != NULL; i++) {
        if (_fstricmp(tkm[i].szKeyName, lpkme->szKeyName) == 0) {
            km2.bShift = lpkme->bShift;
            km2.bControl = lpkme->bShift;
            km2.bAlternate = lpkme->bAlternate;
            km2.vk = lpkme->vk;
            km2.tk = tkm[i].nKeyID;
            KeyMapAdd(&km2);
            break;
        }
    }

    if (tkm[i].szKeyName == NULL) {
        char szMessage[80];

        wsprintf(szMessage, "Unknown TTY key neumonic %s!",
                 (LPSTR)lpkme->szKeyName);
        MessageBox(NULL, szMessage, "Kermit Key Parse Error",
                   MB_OK | MB_ICONEXCLAMATION);

        return FALSE;
    }

    return TRUE;
}

/*----------------------------------------------------------------------------*/
int NEAR GetRadioButton
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Scan a set of button controls looking for the first button that is        */
/*  selected.  Return the ID of the first selected button or 0 if none        */
/*  are selected.                                                             */
/*                                                                            */
(HWND hDlg,             /* Handle to dialog containing buttons.               */
 int nIDFirstButton,    /* Control ID of first button in set.                 */
 int nIDLastButton)     /* Control ID of last button in set.                  */
/*----------------------------------------------------------------------------*/

{
    int i;

    for (i = nIDFirstButton; i <= nIDLastButton; i++) {
        if (IsDlgButtonChecked(hDlg, i))
            return(i);
    }

    return(0);
}

/*----------------------------------------------------------------------------*/
BOOL DLLEXPORT CALLBACK AboutDlgProc
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Terminal setup dialog procedure.                                          */
/*                                                                            */
(HWND       hDlg,      /* Handle to termainal setup dialog                    */
 UINT       message,   /* Mesage number.                                      */
 WPARAM     wParam,    /* First paramter.                                     */
 LPARAM     X(lParam)) /* Second parameter.                                   */
/*----------------------------------------------------------------------------*/

{
    char szWork[512];

    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    EndDialog(hDlg, TRUE);
                    break;

                default:
                    return(FALSE);
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

/*----------------------------------------------------------------------------*/
BOOL NEAR TermDlgInit
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Initialize the terminal setup dialog by prefilling controls.              */
/*                                                                            */
(HWND hDlg)     /* Handle to terminal setup dialog to initialize.             */
/*----------------------------------------------------------------------------*/

{
    HANDLE hEmul;

    hEmul = GetProp(hDlg, "hEmul");
    npTCB = (TCB *)LocalLock(hEmul);
    if (!npTCB)
        return(FALSE);

    CheckDlgButton(hDlg, IDD_NEWLINE, npTCB->TermSet.NewLine);
    CheckDlgButton(hDlg, IDD_LOCALECHO, npTCB->TermSet.LocalEcho);
    CheckDlgButton(hDlg, IDD_LINEWRAP, npTCB->TermSet.LineWrap);

    LocalUnlock(hEmul);
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL NEAR TermDlgTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Terminate the terminal setup dialog by validating and saving the          */
/*  values in the controls.  Return TRUE only if all controls contain         */
/*  valid values.                                                             */
/*                                                                            */
(HWND hDlg)     /* Handle to terminal setup dialog to terminate.              */
/*----------------------------------------------------------------------------*/

{
    HANDLE hEmul;

    hEmul = GetProp(hDlg, "hEmul");
    npTCB = (TCB *)LocalLock(hEmul);
    if (!npTCB)
        return(FALSE);

    npTCB->TermSet.NewLine = (WORD)IsDlgButtonChecked(hDlg, IDD_NEWLINE);
    npTCB->TermSet.LocalEcho = (WORD)IsDlgButtonChecked(hDlg, IDD_LOCALECHO);
    npTCB->TermSet.LineWrap = (WORD)IsDlgButtonChecked(hDlg, IDD_LINEWRAP);

    LocalUnlock(hEmul);
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
BOOL DLLEXPORT CALLBACK TermDlgProc
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Terminal setup dialog procedure.                                          */
/*                                                                            */
(HWND       hDlg,      /* Handle to termainal setup dialog                    */
 UINT       message,   /* Mesage number.                                      */
 WPARAM     wParam,    /* First paramter.                                     */
 LPARAM     lParam)    /* Second parameter.                                   */
/*----------------------------------------------------------------------------*/

{
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDD_OK:
                    if (!TermDlgTerm(hDlg))
                        break;

                case IDD_CANCEL:
                    RemoveProp(hDlg, "hEmul");
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_ABOUT:
                    DialogBoxParam(hLibrary, "AboutDlgBox", hDlg,
                                   AboutDlgProc, NULL);
                    break;

                default:
                    return(FALSE);
                    break;
            }
            break;

    case WM_INITDIALOG:
#ifdef _WIN32
        SetProp(hDlg, "hEmul", (HANDLE)lParam);
#else
        SetProp(hDlg, "hEmul", (HANDLE)LOWORD(lParam));
#endif
            return(TermDlgInit(hDlg));

        default:
            return(FALSE);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------------*/
extern "C" int DLLEXPORT CALLBACK SetupTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Invokes the terminal setup dialog.                                        */
/*                                                                            */
(HANDLE hEmul)  /* Handle to callers terminal window.                         */
/*----------------------------------------------------------------------------*/

{
    int nResult;
    int i;

    npTCB = (TCB *)LocalLock(hEmul);

#ifdef _WIN32
    nResult = DialogBoxParam(hLibrary, "TermDlgBox", npTCB->hAppWnd,
                 TermDlgProc, (LPARAM)hEmul);
#else
    nResult = DialogBoxParam(hLibrary, "TermDlgBox", npTCB->hAppWnd,
                 TermDlgProc, MAKELONG(hEmul, 0));
#endif

    LocalUnlock(hEmul);

    if (!KeyMapParse("KERMTTY.KEY"))
        for (i = 0; km[i].vk != 0; i++)
            KeyMapAdd(&(km[i]));

    return(nResult);
}

/*----------------------------------------------------------------------------*/
extern "C" BOOL DLLEXPORT CALLBACK GetTermConfig
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Retrieve the current terminal configuration and copy it to the            */
/*  requested memory area.  Return TRUE if successful.                        */
/*                                                                            */
(HANDLE hEmul,      /* Handle to callers terminal window.                     */
 LPSTR lpConfig)    /* Pointer to memory area to fill with terminal config.   */
/*----------------------------------------------------------------------------*/

{
    if (lpConfig == NULL)
        return(FALSE);

    npTCB = (TCB *)LocalLock(hEmul);
    if (!npTCB)
        return(FALSE);

    _fmemcpy(lpConfig, &npTCB->TermSet, sizeof(TermBlk));

    LocalUnlock(hEmul);

    return(TRUE);
}

/*----------------------------------------------------------------------------*/
extern "C" HANDLE DLLEXPORT CALLBACK OpenTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Perform all terminal window intialization required.  Save the             */
/*  configuration information passed, save the callback proc addresses,       */
/*  and initialize local work variables.                                      */
/*                                                                            */
(HWND hAppWnd,              /* Handle to callers terminal window.             */
 LPSTR lpConfig,            /* Pointer to terminal config.                    */
 FARPROC FAR * APIProcs)    /* Array of callback proc addresses.              */
/*----------------------------------------------------------------------------*/

{
    HANDLE hEmul;
    int i;

    hEmul = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(TCB));
    npTCB = (TCB *)LocalLock(hEmul);

    _fmemcpy(&npTCB->TermSet, &TermDef, sizeof(TermBlk));

    if (lpConfig != NULL)
        _fmemcpy(&npTCB->TermSet, lpConfig, sizeof(TermBlk));

    for (i = 0; i < APIPROCCNT; i++)
        npTCB->APIProcs[i] = APIProcs[i];

    npTCB->hAppWnd = hAppWnd;
    npTCB->nCurRow = 0; npTCB->nCurCol = 0;
    npTCB->nSavRow = 0; npTCB->nSavCol = 0;

    LocalUnlock(hEmul);

    DrawEmul("Teletype Emulation");

    KeyMapClear();
    for (i = 0; km[i].vk != 0; i++)
        KeyMapAdd(&(km[i]));

    return(hEmul);
}

/*----------------------------------------------------------------------------*/
extern "C" VOID DLLEXPORT CALLBACK CloseTerm
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Free all resources allocated to the callers terminal window in            */
/*  preparation for the windows destruction.                                  */
/*                                                                            */
(HANDLE hEmul)  /* Handle to callers terminal window.                         */
/*----------------------------------------------------------------------------*/

{
    LocalFree(hEmul);
}
